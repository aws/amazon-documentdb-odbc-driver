# ODBC Support and Limitations

## Connection Attributes
| Connection attribute | Default | Support Value Change|
|--------|------|-------|
|SQL_ATTR_CONNECTION_DEAD| - | No |
|SQL_ATTR_LOGIN_TIMEOUT| 0 | Yes |

## Statements Attributes
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
supports the use of SQLPrepare. However, the use of parameters in queries (values left as ?) is not supported in SQLPrepare, SQLExecute and SQLExecDirect. 

## PowerBI Not Been Able to Load DocumentDB ODBC driver library

If you downloaded Power BI Desktop from the Microsoft Store, you might be unable to use Amazon DocumentDB ODBC driver because of an driver not been able to load issue. To address it, you have to do download Power BI Desktop from the [Download Center](https://www.microsoft.com/download/details.aspx?id=58494) instead of Microsoft Store.
