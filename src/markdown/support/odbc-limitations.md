# ODBC Support and Limitations

// TODO create user Documentation

//https://bitquill.atlassian.net/browse/AD-682

Table of statement attributes supported by the Amazon DocumentDB ODBC driver.\
Related function: `SQLSetStmtAttr`
| Statement attribute | Default | Support Value Change|
|--------|------|-------|
|SQL_ATTR_PARAM_BIND_OFFSET_PTR| |  |
|SQL_ATTR_PARAM_BIND_TYPE| |  |
|SQL_ATTR_PARAM_OPERATION_PTR| | |
|SQL_ATTR_PARAM_STATUS_PTR| | |
|SQL_ATTR_PARAMS_PROCESSED_PTR| | |
|SQL_ATTR_PARAMSET_SIZE| | | 
|SQL_ATTR_ROW_ARRAY_SIZE| 1 | No | 
|SQL_ATTR_ROW_BIND_OFFSET_PTR| | |
|SQL_ATTR_ROW_BIND_TYPE| | |
|SQL_ATTR_ROW_OPERATION_PTR| | |
|SQL_ATTR_ROW_STATUS_PTR| | |
|SQL_ATTR_ROWS_FETCHED_PTR| | |

## SQLPrepare,SQLExecute and SQLExecDirect

To support BI tools that may use the SQLPrepare interface in auto-generated queries, the driver
supports the use of SQLPrepare. However, the use of parameters in queries (values left as ?) is not supported SQLExecute and SQLExecDirect. 


