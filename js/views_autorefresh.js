(function ($) {
  // START jQuery

  // Carry over or default.
  Backdrop.views_autorefresh = Backdrop.views_autorefresh || {};

  Backdrop.behaviors.views_autorefresh = {
    attach: function(context, settings) {
      // Close timers on page unload.
      window.addEventListener('unload', function(event) {
        $.each(Backdrop.settings.views_autorefresh, function(index, entry) {
          clearTimeout(entry.timer);
        });
      });

      if (Backdrop.settings && Backdrop.settings.views && Backdrop.settings.views.ajaxViews) {
        var ajax_path = Backdrop.settings.views.ajax_path;

        // If there are multiple views this might've ended up showing up multiple times.
        if (ajax_path.constructor.toString().indexOf('Array') != -1) {
          ajax_path = ajax_path[0];
        }

        $.each(Backdrop.settings.views.ajaxViews, function(i, settings) {
          var view_name_id = settings.view_name + '-' + settings.view_display_id;

          // Carry over or default.
          Backdrop.views_autorefresh[view_name_id] = Backdrop.views_autorefresh[view_name_id] || {};

          if (!(view_name_id in Backdrop.settings.views_autorefresh)) {
            // This view has not got views_autorefresh behavior enabled, so exit
            // early to avoid potential errors.
            return;
          }

          var viewDom = '.view-dom-id-' + settings.view_dom_id;

          if (!$(viewDom).size()) {
            // Backward compatibility: if 'views-view.tpl.php' is old and doesn't
            // contain the 'view-dom-id-#' class, we fall back to the old way of
            // locating the view:
            viewDom = '.view-id-' + settings.view_name + '.view-display-id-' + settings.view_display_id;
          }

          $(viewDom).filter(':not(.views-autorefresh-processed)')
            // Don't attach to nested views. Doing so would attach multiple behaviors
            // to a given element.
            .filter(function() {
              // If there is at least one parent with a view class, this view
              // is nested (e.g., an attachment). Bail.
              return !$(this).parents('.view').size();
            })
            .each(function() {
              // Set a reference that will work in subsequent calls.
              Backdrop.settings.views_autorefresh[view_name_id].target = this;

              // Stop the timer when a user clicks or changes a form element.
              $('input, select, textarea', Backdrop.settings.views_autorefresh[view_name_id].target)
                .click(function () {
                  if (!Backdrop.settings.views_autorefresh[view_name_id].incremental) {
                    clearTimeout(Backdrop.settings.views_autorefresh[view_name_id].timer);
                  }
                })
                .change(function () {
                  // Duplicate action.
                  $(this).click();
                });

              $(this)
                .addClass('views-autorefresh-processed')
                // Process pager, tablesort, and attachment summary links.
                .find('.auto-refresh a')
                .each(function () {
                  var viewData = {
                    'js': '1',
                    'autorefresh': true
                  };
                  var href = $(this).attr('href');

                  // Recover lost base_path if necessary.
                  settings.view_base_path = settings.view_base_path || settings.view_path;

                  // Construct an object using the settings defaults and then overriding
                  // with data specific to the link.
                  $.extend(
                    viewData,
                    Backdrop.Views.parseQueryString(href),
                    // Extract argument data from the URL.
                    Backdrop.Views.parseViewArgs(href, settings.view_base_path),
                    // Settings must be used last to avoid sending url aliases to the server.
                    settings
                  );

                  Backdrop.settings.views_autorefresh[view_name_id].view_args = viewData.view_args;
                  Backdrop.settings.views_autorefresh[view_name_id].anchor = this;

                  // Setup the click response with Backdrop.ajax.
                  var element_settings = {
                    url: ajax_path,
                    event: 'click',
                    selector: view_name_id,
                    submit: viewData
                  };

                  Backdrop.settings.views_autorefresh[view_name_id].ajax = new Backdrop.ajax(view_name_id, Backdrop.settings.views_autorefresh[view_name_id].anchor, element_settings);

                  // Optionally trigger refresh only once per load.
                  if (
                    Backdrop.settings.views_autorefresh[view_name_id].trigger_onload &&
                    !Backdrop.views_autorefresh[view_name_id].loaded
                  ) {
                    Backdrop.views_autorefresh[view_name_id].loaded = true;

                    // Trigger custom event on any plugin that needs to do extra work.
                    $(Backdrop.settings.views_autorefresh[view_name_id].target).trigger('autorefresh_onload', view_name_id);

                    Backdrop.views_autorefresh.refresh(view_name_id);
                  }
                  // Activate refresh timer if not using nodejs.
                  else if (!Backdrop.settings.views_autorefresh[view_name_id].nodejs) {
                    clearTimeout(Backdrop.settings.views_autorefresh[view_name_id].timer);
                    Backdrop.views_autorefresh.timer(view_name_id);
                  }
                });
            });
        });
      }
    }
  };

  Backdrop.views_autorefresh.timer = function(view_name_id) {
    Backdrop.settings.views_autorefresh[view_name_id].timer = setTimeout(function() {
      clearTimeout(Backdrop.settings.views_autorefresh[view_name_id].timer);
      Backdrop.views_autorefresh.refresh(view_name_id);
    }, Backdrop.settings.views_autorefresh[view_name_id].interval);
  };

  Backdrop.views_autorefresh.refresh = function(view_name_id) {
    // Turn off new items class.
    $('.views-autorefresh-new', Backdrop.settings.views_autorefresh[view_name_id].target).removeClass('views-autorefresh-new');

    var viewData = Backdrop.settings.views_autorefresh[view_name_id].ajax.submit;

    // Handle secondary view for incremental refresh.
    // @url http://stackoverflow.com/questions/122102/what-is-the-most-efficient-way-to-clone-a-javascript-object
    if (Backdrop.settings.views_autorefresh[view_name_id].incremental) {
      if (!viewData.original_view_data) {
        viewData.original_view_data = $.extend(true, {}, viewData);
      }

      viewData.view_args = (Backdrop.settings.views_autorefresh[view_name_id].view_args.length ? Backdrop.settings.views_autorefresh[view_name_id].view_args + '/' : '') + Backdrop.settings.views_autorefresh[view_name_id].timestamp;
      viewData.view_base_path = Backdrop.settings.views_autorefresh[view_name_id].incremental.view_base_path;
      viewData.view_display_id = Backdrop.settings.views_autorefresh[view_name_id].incremental.view_display_id;
      viewData.view_name = Backdrop.settings.views_autorefresh[view_name_id].incremental.view_name;
    }

    // Overwrite variable.
    Backdrop.settings.views_autorefresh[view_name_id].ajax.submit = viewData;

    // If there's a ping URL, hit it first.
    if (Backdrop.settings.views_autorefresh[view_name_id].ping) {
      var pingData = { 'timestamp': Backdrop.settings.views_autorefresh[view_name_id].timestamp };

      $.extend(pingData, Backdrop.settings.views_autorefresh[view_name_id].ping.ping_args);
      $.ajax({
        url: Backdrop.settings.basePath + Backdrop.settings.views_autorefresh[view_name_id].ping.ping_base_path,
        data: pingData,
        success: function(response) {
          if (response.pong && parseInt(response.pong) > 0) {
            $(Backdrop.settings.views_autorefresh[view_name_id].anchor).trigger('click');
            // Trigger custom event on any plugin that needs to do extra work.
            $(Backdrop.settings.views_autorefresh[view_name_id].target).trigger('autorefresh_ping', parseInt(response.pong));
          }
          else if (!Backdrop.settings.views_autorefresh[view_name_id].nodejs) {
            Backdrop.views_autorefresh.timer(view_name_id);
          }
        },
        error: function(xhr) {},
        dataType: 'json'
      });
    }
    else {
      $(Backdrop.settings.views_autorefresh[view_name_id].anchor).trigger('click');
    }
  };

  Backdrop.ajax.prototype.commands.viewsAutoRefreshTriggerUpdate = function (ajax, response, status) {
    // Trigger custom event on any plugin that needs to do extra work.
    $(response.selector).trigger('autorefresh_update', response.timestamp);
  };

  // @url http://stackoverflow.com/questions/1394020/jquery-each-backwards
  jQuery.fn.reverse = [].reverse;

  Backdrop.ajax.prototype.commands.viewsAutoRefreshIncremental = function (ajax, response, status) {
    if (response.data) {
      // jQuery removes script tags, so let's mask them now and later unmask.
      // @url http://stackoverflow.com/questions/4430707/trying-to-select-script-tags-from-a-jquery-ajax-get-response/4432347#4432347
      response.data = response.data.replace(/<(\/?)script([^>]*)>/gi, '<$1scripttag$2>');

      var $view = $(response.selector);
      var view_name_id = response.view_name_id;
      var emptySelector = Backdrop.settings.views_autorefresh[view_name_id].incremental.emptySelector || '.view-empty';
      var sourceSelector = Backdrop.settings.views_autorefresh[view_name_id].incremental.sourceSelector || '.view-content';
      var $source = $(response.data).find(sourceSelector).not(sourceSelector + ' ' + sourceSelector).children();

      if ($source.size() > 0 && $(emptySelector, $source).size() <= 0) {
        var targetSelector = Backdrop.settings.views_autorefresh[view_name_id].incremental.targetSelector || '.view-content';
        var $target = $view.find(targetSelector).not(targetSelector + ' ' + targetSelector);

        // If initial view was empty, remove the empty divs then add the target div.
        if ($target.size() == 0) {
          var afterSelector = Backdrop.settings.views_autorefresh[view_name_id].incremental.afterSelector || '.view-header';
          var targetStructure = Backdrop.settings.views_autorefresh[view_name_id].incremental.targetStructure || '<div class="view-content"></div>';

          if ($(emptySelector, $view).size() > 0) {
            // Replace empty div with content.
            $(emptySelector, $view).replaceWith(targetStructure);
          }
          else if ($(afterSelector, $view).size() > 0) {
            // Insert content after given div.
            $view.find(afterSelector).not(targetSelector + ' ' + afterSelector).after(targetStructure);
          }
          else {
            // Insert content as first child of view div.
            $view.prepend(targetStructure);
          }

          // Now that it's inserted, find it for manipulation.
          $target = $view.find(targetSelector).not(targetSelector + ' ' + targetSelector);
        }

        // Remove first, last row classes from items.
        var firstClass = Backdrop.settings.views_autorefresh[view_name_id].incremental.firstClass || 'views-row-first';
        var lastClass = Backdrop.settings.views_autorefresh[view_name_id].incremental.lastClass || 'views-row-last';

        $target.children().removeClass(firstClass);
        $source.removeClass(lastClass);

        // Adjust even-odd classes.
        var oddClass = Backdrop.settings.views_autorefresh[view_name_id].incremental.oddClass || 'views-row-odd';
        var evenClass = Backdrop.settings.views_autorefresh[view_name_id].incremental.evenClass || 'views-row-even';
        var oddness = $target.children(':first').hasClass(oddClass);

        $source.filter('.' + oddClass + ', .' + evenClass).reverse().each(function() {
          $(this).removeClass(oddClass + ' ' + evenClass).addClass(oddness ? evenClass : oddClass);
          oddness = !oddness;
        });

        // Add the new items to the view.
        // Put scripts back first.
        $source.each(function() {
          $target.prepend($(this)[0].outerHTML.replace(/<(\/?)scripttag([^>]*)>/gi, '<$1script$2>'));
        });

        // Adjust row number classes.
        var rowClassPrefix = Backdrop.settings.views_autorefresh[view_name_id].incremental.rowClassPrefix || 'views-row-';
        var rowRegex = new RegExp('views-row-(\\d+)');

        $target.children().each(function(i) {
          $(this).attr('class', $(this).attr('class').replace(rowRegex, rowClassPrefix + (i+1)));
        });

        // Trigger custom event on any plugin that needs to do extra work.
        $view.trigger('autorefresh_incremental', $source.size());
      }

      // Reactivate refresh timer if not using nodejs.
      if (!Backdrop.settings.views_autorefresh[view_name_id].nodejs) {
        Backdrop.views_autorefresh.timer(view_name_id, $('.auto-refresh a', $view), $view);
      }

      // Attach behaviors
      Backdrop.attachBehaviors($view);
    }
  };

  Backdrop.Nodejs = Backdrop.Nodejs || { callbacks: {} };

  // Callback for nodejs message.
  Backdrop.Nodejs.callbacks.viewsAutoRefresh = {
    callback: function (message) {
      var view_name_id = message['view_name_id'];
      Backdrop.views_autorefresh.refresh(view_name_id);
    }
  };

  // END jQuery
})(jQuery);
