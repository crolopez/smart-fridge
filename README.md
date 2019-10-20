[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# Smart Fridge

The purpose of this food inventory management system is to facilitate knowledge of what products are stored in the refrigerator or pantry from anywhere and at any time, allowing to manage self-generated shopping lists and to launch notifications reminding the user of the scarcity or caducity of certain products when passing near establishments where they can be purchased.

This repository includes the system backend, which is designed to be installed on an embedded system such as a Raspberry Pi. You can find the frontend of the project in [this repository](https://github.com/crolopez/smart-fridge-app).

## How to deploy

To deploy the project you simply have to clone, compile and install it.

``` BASH
# git clone https://github.com/crolopez/smart-fridge
# cd smart-fridge
# make
# make install
```

These steps will perform a default installation of the system, including the service for Systemd if applicable. This deployment process can be configured through the makefile parameters. These are:

| Option | Description | Allowed value | Default value|
|--|--|--|--|
| DEBUG | Enables debug mode | YES, NO | NO |
| TEST | Compiles the testing binaries | YES, NO | NO |
| PARALLEL_DEPS | Compiles the dependencies in parallel | A valid thread number | None |
| ENABLE_LDAP | Enables OpenLDAP integration | YES, NO | NO |
| DB | Launches database interface | None | None |
| ENABLE_CAM | Compile with camera support | YES, NO | NO |
| INSTALL_PATH | Set the install directory | A valid installation path | /opt |

Example of use:

``` BASH
make TEST=YES DEBUG=YES PARALLEL_DEPS=4 ENABLE_CAM=YES PARALLEL_DEPS=8
```

## Dependencies

If the compilation fails, it is possible that you need to install some extra-dependency as `gcc` or `make`. In order to facilitate deployment and avoid issues related to compatibilities between libraries, the main dependencies have been embedded, so they will be self-compiled. These are:

- [**cJSON 1.7.7**](https://github.com/DaveGamble/cJSON)
- [**libCurl 7.59.0**](https://curl.haxx.se/libcurl/)
- [**OpenLDAP 2.4**](https://www.openldap.org/)
- [**OpenSSL 1.1.0h**](https://www.openssl.org/t)
- [**Sqlite 3**](https://www.sqlite.org/index.html)
- [**libYAML 0.1.7**](https://github.com/yaml/libyaml)
- [**ZBar 0.10**](http://zbar.sourceforge.net/)
- [**zlib 1.2.11**](https://www.zlib.net/)

## Configuration

Once installed, it is necessary to configure the system through the `config.yaml` file located in the installation root. These parameters are:

| Node | Component |  Application | Description | Allowed value | Default value
|--|--|--|--|--|--|
|reader|id_queue|sleep| Waiting time between insertions in the code queue |A valid interval of seconds|1|
|reader|id_queue|size| Code queue size | A logical size |100|
|reader|product_queue|sleep| Waiting time between insertions in the product queue|A valid interval of seconds|1|
|reader|product_queue|size| Product queue size | A logical size |100|
|reader|test_mode|sleep| Waiting time between test mode readings |A valid interval of seconds|2|
|reader|test_mode|infinite| Sets test mode to not stop when finished reading the file |yes, no|yes|
|reader|socket_mode|port| Sets the listening port in socket mode | A valid port |7383|
|reader|code_reader|device| Sets the location of the special device referenced by the camera driver if used | A valid device path|/dev/video0|
|reader|logs|location| Log file of the input reader process | A valid log file |/opt/smart-fridge/logs/reader.log|
|database|ports|internal| Sets the database listening port | A valid port |2187|
|database|headers|internal| Header tag of the database messages | Any tag |!#+|
|database|addresses|internal| Sets the listening address of the database, which allows us to host both processes on different machines | A valid address |127.0.0.1|
|database|logs|location| Log file of the database management process| A valid log file|/opt/smart-fridge/logs/database.log|

## Execute

To start service on a system with Systemd:

``` BASH
systemctl start smart-fridge
```

To set up automatic startup:

``` BASH
systemctl enable smart-fridge
```

If you are using a system without Systemd:

``` BASH
/opt/bin/sf-control start
```
