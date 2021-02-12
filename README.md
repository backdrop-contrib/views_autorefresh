# Views Auto-Refresh

A module to refresh a view after a specified time interval. 

Includes advanced options to avoid reloading the whole page, and to avoid
causing a full Backdrop bootstrap at each refresh.

The easiest way to use this module is to add a Header of type
*Global: Autorefresh* to your view via Views UI.

Additionally this module ships with *Global: Request time* - an area that
displays the time the view has been fetched from the server (with configurable
date format and text).

## Installation

Install this module using the [official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/views_autorefresh/issues).

**Known issues:** the current maintainer never used any of the advanced methods
for refreshing (ping or node.js), so their functionality hasn't been tested with
Backdrop. The Node.js integration module hasn't been ported to Backdrop (yet).

## Current maintainers

* [Indigoxela](https://github.com/indigoxela)

## Credits

* Original Drupal author: [Karim Ratib (infojunkie)](https://www.drupal.org/u/infojunkie)
* Current Drupal maintainer: [Patrick Waller (firewaller)](https://www.drupal.org/u/firewaller)

Ported to Backdrop by Indigoxela

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
