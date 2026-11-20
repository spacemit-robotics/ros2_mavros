Mavros license variants
=======================

Mavros is available triple-licensed as [BSD][bsd], [GPLv3][gpl] and [LGPLv3][lgpl].
Its use is compatible with any of these licenses.
However, contributions to the upstream repository must be made
available under all three licenses and therefore have
to be compatible to BSD, GPLv3 and LGPLv3.


[lgpl]: https://www.gnu.org/licenses/lgpl.html
[gpl]: https://www.gnu.org/licenses/gpl.html
[bsd]: https://github.com/mavlink/mavros/blob/master/LICENSE-BSD.txt

## License scope in this repository

Code imported from MAVROS in `libmavconn/`, `mavros/`, `mavros_msgs/`,
and `mavros_extras/` retains its original license terms and copyright notices.

The DDS adapters under `dds/` use Apache-2.0; the license text is in
`dds/LICENSE`. Third-party and generated files retain their original notices.
