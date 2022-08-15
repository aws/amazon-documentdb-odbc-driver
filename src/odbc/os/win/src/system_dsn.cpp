/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "documentdb/odbc/config/connection_string_parser.h"
#include "documentdb/odbc/diagnostic/diagnosable_adapter.h"
#include "documentdb/odbc/dsn_config.h"
#include "documentdb/odbc/log.h"
#include "documentdb/odbc/odbc_error.h"
#include "documentdb/odbc/system/odbc_constants.h"
#include "documentdb/odbc/system/ui/dsn_configuration_window.h"
#include "documentdb/odbc/system/ui/window.h"
#include "documentdb/odbc/utility.h"

using documentdb::odbc::config::Configuration;
using documentdb::odbc::utility::FromUtf8;

#ifdef _WIN32
bool DisplayConnectionWindow(void* windowParent, Configuration& config) {
  using namespace documentdb::odbc::system::ui;

  HWND hwndParent = (HWND)windowParent;

  if (!hwndParent)
    return true;

  try {
    Window parent(hwndParent);

    DsnConfigurationWindow window(&parent, config);

    window.Create();

    window.Show();
    window.Update();

    return ProcessMessages(window) == Result::OK;
  } catch (const documentdb::odbc::DocumentDbError& err) {
    std::wstringstream buf;

    buf << L"Message: " << err.GetText() << L", Code: " << err.GetCode();

    MessageBox(NULL, buf.str().c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);

    SQLPostInstallerError(err.GetCode(), FromUtf8(err.GetText()).c_str());
  }

  return false;
}
#endif

/**
 * Register DSN with specified configuration.
 *
 * @param config Configuration.
 * @param driver Driver.
 * @return True on success and false on fail.
 */
bool InternalRegisterDsn(const Configuration& config, LPCSTR driver) {
  using namespace documentdb::odbc::config;
  using documentdb::odbc::common::LexicalCast;

  typedef Configuration::ArgumentMap ArgMap;

  const char* dsn = config.GetDsn().c_str();

  DocumentDbError error;
  try {
    if (!RegisterDsn(config, driver, error))
      throw error;
    return true;
  } catch (documentdb::odbc::DocumentDbError& err) {
    std::wstring errText = FromUtf8(err.GetText());
    MessageBox(NULL, errText.c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);

    SQLPostInstallerError(err.GetCode(), errText.c_str());
  }

  return false;
}

/**
 * Unregister specified DSN.
 *
 * @param dsn DSN name.
 * @return True on success and false on fail.
 */
bool InternalUnregisterDsn(const std::string& dsn) {
  DocumentDbError error;
  try {
    if (UnregisterDsn(dsn, error))
      throw error;

    return true;
  } catch (documentdb::odbc::DocumentDbError& err) {
    std::wstring errText = FromUtf8(err.GetText());
    MessageBox(NULL, errText.c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);

    SQLPostInstallerError(err.GetCode(), errText.c_str());
  }

  return false;
}

BOOL INSTAPI ConfigDSN(HWND hwndParent, WORD req, LPCSTR driver,
                       LPCSTR attributes) {
  using namespace documentdb::odbc;

  LOG_MSG("ConfigDSN called");

  Configuration config;

  LOG_MSG("Attributes: " << attributes);

  config::ConnectionStringParser parser(config);

  diagnostic::DiagnosticRecordStorage diag;

  parser.ParseConfigAttributes(attributes, &diag);

  if (!SQLValidDSN(utility::FromUtf8(config.GetDsn()).c_str()))
    return FALSE;

  LOG_MSG("Driver: " << driver);
  LOG_MSG("DSN: " << config.GetDsn());

  switch (req) {
    case ODBC_ADD_DSN: {
      LOG_MSG("ODBC_ADD_DSN");

#ifdef _WIN32
      if (!DisplayConnectionWindow(hwndParent, config))
        return FALSE;
#endif  // _WIN32

      if (!InternalRegisterDsn(config, driver))
        return FALSE;

      break;
    }

    case ODBC_CONFIG_DSN: {
      LOG_MSG("ODBC_CONFIG_DSN");

      std::string dsn = config.GetDsn();

      Configuration loaded(config);

      ReadDsnConfiguration(dsn.c_str(), loaded, &diag);

#ifdef _WIN32
      if (!DisplayConnectionWindow(hwndParent, loaded))
        return FALSE;
#endif  // _WIN32

      if (!InternalRegisterDsn(loaded, driver))
        return FALSE;

      if (loaded.GetDsn() != dsn && !InternalUnregisterDsn(dsn.c_str()))
        return FALSE;

      break;
    }

    case ODBC_REMOVE_DSN: {
      LOG_MSG("ODBC_REMOVE_DSN");

      if (!InternalUnregisterDsn(config.GetDsn().c_str()))
        return FALSE;

      break;
    }

    default:
      return FALSE;
  }

  return TRUE;
}
