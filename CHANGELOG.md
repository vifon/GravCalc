# GravCalc Changelog

## v1.13

- some optimizations
- Pebble Time version uploaded to the appstore

## v1.12

- the buttons are concave: they cursor is "repelled" from their edges

## v1.11

- ready for Pebble Time!
- improved the button sizing

## v1.10
- division has the precision just like before the `v1.8` update (when
  possible without overflows)

## v1.9
- overflow detection and notification

## v1.8
- many small UI tweaks
- many big internal API tweaks
- a few documentation changes
- new icons (thanks, @beamerkun)
- multiplication and division can handle greater numbers, at the cost
  of some fractional precision for division

## v1.7
- automatic accelerometer calibration
- bugfix: the number parser was reading garbage data

## v1.6
- bugfix: exponent was broken since `v1.4`

## v1.5
- bugfix: `+` behaved just like `-` at the beginning of a number (both
  performed a negation)

## v1.4
- fraction support (up to 2 decimal places)

## v1.3
- the calculation result is automatically pushed to the stack

## v1.2
- negation button removed
- negation is performed by pressing the minus sign
- exponent added

## v1.1
- added a menu icon

## v1.0
- the first version
