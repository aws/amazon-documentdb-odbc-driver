# Windows - Configuring a DSN

## Add DSN

1. Run `ODBC Data Sources (64-bit)` or `ODBC Data Sources (32-bit)`. Click on the `System DSN` or `User DSN` tab then click `Add...`.

 <img src="img/win_user_dsn.png" width="60%">

2. Select `Amazon Timestream ODBC Driver` and click on `Finish`.

<img src="img/win_user_dsn_select_driver.png" width="50%">

3. The DSN Setup window will open.

 <img src="img/win_user_dsn_configure_options.png" width="50%">

4. Update the values of the configuration options. See [Configuration Options](./configuration_options.md) for more details.
5. Click `Test` to verify connectivity; you will get a `Connection successful` message if your configuration is correct.
6. Click `OK` to save the DSN values. 
7. You will find this newly added DSN in the `System DSN` or `User DSN` list.

 <img src="img/win_user_dsn_list.png" width="60%">
