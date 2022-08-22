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

#include "documentdb/odbc/system/ui/dsn_configuration_window.h"

#include <ShlObj_core.h>
#include <Shlwapi.h>
#include <Windowsx.h>

#include "documentdb/odbc/config/config_tools.h"
#include "documentdb/odbc/config/configuration.h"
#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/log.h"
#include "documentdb/odbc/log_level.h"
#include "documentdb/odbc/read_preference.h"
#include "documentdb/odbc/scan_method.h"

#define VALIDATE_FOR_TEST()                                                 \
  nameEdit->HasText() && databaseEdit->HasText() && hostnameEdit->HasText() \
      && portEdit->HasText() && userEdit->HasText() && passwordEdit->HasText()

#define VALIDATE_FOR_SAVE()                                                   \
  nameEdit->HasText() && databaseEdit->HasText() && hostnameEdit->HasText() \
      && portEdit->HasText()

namespace documentdb {
namespace odbc {
namespace system {
namespace ui {
DsnConfigurationWindow::DsnConfigurationWindow(Window* parent,
                                               config::Configuration& config)
    : CustomWindow(parent, L"DocumentDbConfigureDsn",
                   L"Configure Amazon DocumentDB DSN"),
      width(500),
      height(540),
      connectionSettingsGroupBox(),
      tlsCheckBox(),
      nameLabel(),
      nameEdit(),
      nameBalloon(),
      shownNameBalloon(false),
      scanMethodLabel(),
      scanMethodComboBox(),
      scanLimitLabel(),
      scanLimitEdit(),
      scanLimitBalloon(),
      shownScanLimitBalloon(false),
      schemaLabel(),
      schemaEdit(),
      schemaBalloon(),
      shownSchemaBalloon(false),
      refreshSchemaCheckBox(),
      schemaGroupControls(),
      sshEnableCheckBox(),
      sshUserLabel(),
      sshUserEdit(),
      sshHostLabel(),
      sshHostEdit(),
      sshPrivateKeyFileLabel(),
      sshPrivateKeyFileEdit(),
      sshPrivateKeyFileBrowseButton(),
      sshStrictHostKeyCheckingCheckBox(),
      sshKnownHostsFileLabel(),
      sshKnownHostsFileEdit(),
      sshKnownHostsFileBrowseButton(),
      sshGroupControls(),
      logLevelLabel(),
      logLevelComboBox(),
      logPathLabel(),
      logPathEdit(),
      logPathBrowseButton(),
      loggingGroupControls(),
      appNameLabel(),
      appNameEdit(),
      readPreferenceLabel(),
      readPreferenceComboBox(),
      replicaSetLabel(),
      replicaSetEdit(),
      retryReadsCheckBox(),
      loginTimeoutSecEdit(),
      loginTimeoutSecLabel(),
      loginTimeoutSecBalloon(),
      shownLoginTimeoutSecBalloon(false),
      defaultFetchSizeLabel(),
      defaultFetchSizeEdit(),
      defaultFetchSizeBalloon(),
      shownDefaultFetchSizeBalloon(false),
      additionalGroupControls(),
      databaseLabel(),
      databaseEdit(),
      databaseBalloon(),
      shownDatabaseBalloon(false),
      hostnameLabel(),
      hostnameEdit(),
      hostnameBalloon(),
      shownHostnameBalloon(false),
      portLabel(),
      portEdit(),
      portBalloon(),
      shownPortBalloon(false),
      saveButton(),
      cancelButton(),
      config(config),
      accepted(false),
      created(false),
      versionLabel(),
      userLabel(),
      userEdit(),
      passwordLabel(),
      passwordEdit(),
      testButton(),
      testSettingsGroupBox(),
      testLabel(),
      tlsAllowInvalidHostnamesCheckBox(),
      tlsCaFileLabel(),
      tlsCaFileEdit(),
      tlsCaFileBrowseButton(),
      tlsGroupControls(),
      tabGroupControls(),
      tabs(),
      tabsGroupBox(),
      prevSelectedTabIndex(TabIndex::Type::TLS) {
  // No-op.
}

DsnConfigurationWindow::~DsnConfigurationWindow() {
  // No-op.
}

void DsnConfigurationWindow::Create() {
  // Finding out parent position.
  RECT parentRect;
  GetWindowRect(parent->GetHandle(), &parentRect);

  // Positioning window to the center of parent window.
  const int posX =
      parentRect.left + (parentRect.right - parentRect.left - width) / 2;
  const int posY =
      parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;

  RECT desiredRect = {posX, posY, posX + width, posY + height};
  AdjustWindowRect(&desiredRect,
                   WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);

  Window::Create(WS_OVERLAPPED | WS_SYSMENU, desiredRect.left, desiredRect.top,
                 desiredRect.right - desiredRect.left,
                 desiredRect.bottom - desiredRect.top, 0);

  if (!handle) {
    std::stringstream buf;

    buf << "Can not create window, error code: " << GetLastError();

    throw DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                          buf.str().c_str());
  }
}

int DsnConfigurationWindow::CreateTabs(int posX, int posY, int sizeX) {
  int rowPos = posY;
  rowPos += INTERVAL;

  tabs = CreateTabControl(posX, rowPos, sizeX, ROW_HEIGHT, L"Tabs", ChildId::TABS);

  tabs->AddTabCtrlItem(TabIndex::Type::TLS, L"TLS");
  tabs->AddTabCtrlItem(TabIndex::Type::SSH_TUNNEL, L"SSH Tunnel");
  tabs->AddTabCtrlItem(TabIndex::Type::SCHEMA, L"Schema");
  tabs->AddTabCtrlItem(TabIndex::Type::LOGGING, L"Logging");
  tabs->AddTabCtrlItem(TabIndex::Type::ADDITIONAL, L"Additional");

  tabsGroupBox =
      CreateGroupBox(posX, rowPos + TAB_CTRL_HEIGHT, sizeX,
                     TAB_GROUP_BOX_HEIGHT, L"", ChildId::TABS_GROUP_BOX);

  rowPos += INTERVAL + ROW_HEIGHT;

  return rowPos - posY;
}

void DsnConfigurationWindow::OnCreate() {
  int groupPosY = MARGIN;
  int groupSizeY = (width) - (2 * MARGIN);

  groupPosY += CreateConnectionSettingsGroup(MARGIN, groupPosY, groupSizeY);

  groupPosY += INTERVAL + CreateTabs(MARGIN, groupPosY, groupSizeY);

  CreateTlsSettingsGroup(MARGIN, groupPosY, groupSizeY);
  CreateSshSettingsGroup(MARGIN, groupPosY, groupSizeY);
  CreateSchemaSettingsGroup(MARGIN, groupPosY, groupSizeY);
  CreateLogSettingsGroup(MARGIN, groupPosY, groupSizeY);
  CreateAdditionalSettingsGroup(MARGIN, groupPosY, groupSizeY);

  SetTabGroupVisible(TabIndex::Type::TLS, true);
  SetTabGroupVisible(TabIndex::Type::SCHEMA, false);
  SetTabGroupVisible(TabIndex::Type::SSH_TUNNEL, false);
  SetTabGroupVisible(TabIndex::Type::LOGGING, false);
  SetTabGroupVisible(TabIndex::Type::ADDITIONAL, false);
  prevSelectedTabIndex = TabIndex::Type::TLS;

  groupPosY += TAB_GROUP_BOX_HEIGHT - ROW_HEIGHT;

  groupPosY +=
      INTERVAL + CreateTestSettingsGroup(MARGIN, groupPosY, groupSizeY);

  int cancelPosX = width - MARGIN - BUTTON_WIDTH;
  int okPosX = cancelPosX - INTERVAL - BUTTON_WIDTH;
  int versionSizeX = width - (BUTTON_WIDTH * 2) - (INTERVAL * 4);
  int versionPosX = MARGIN + INTERVAL;

  std::wstringstream versionString;
  versionString << L"Version: " << DRIVER_VERSION;
  versionLabel =
      CreateLabel(versionPosX, groupPosY, versionSizeX, BUTTON_HEIGHT,
                  versionString.str(), ChildId::VERSION_LABEL);
  saveButton = CreateButton(okPosX, groupPosY, BUTTON_WIDTH, BUTTON_HEIGHT, L"Save",
                          ChildId::OK_BUTTON);
  cancelButton = CreateButton(cancelPosX, groupPosY, BUTTON_WIDTH,
                              BUTTON_HEIGHT, L"Cancel", ChildId::CANCEL_BUTTON);

  // check whether the required fields are filled. If not, Ok button is
  // disabled.
  created = true;
  saveButton->SetEnabled(VALIDATE_FOR_SAVE());
  testButton->SetEnabled(VALIDATE_FOR_TEST());
}

int DsnConfigurationWindow::CreateConnectionSettingsGroup(int posX, int posY,
                                                          int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  std::wstring wVal = utility::FromUtf8(config.GetDsn());
  nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                          L"Data Source Name*:", ChildId::NAME_LABEL);
  nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::NAME_EDIT);
  nameBalloon =
      CreateBalloon(L"Required", L"DSN is a required field.", TTI_ERROR);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetHostname());
  hostnameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Hostname*:", ChildId::HOST_NAME_LABEL);
  hostnameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::HOST_NAME_EDIT);
  hostnameBalloon =
      CreateBalloon(L"Required", L"Hostname is a required field.", TTI_ERROR);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetPort());
  portLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"Port*:",
                          ChildId::PORT_LABEL);
  portEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::PORT_EDIT, ES_NUMBER);
  portBalloon =
      CreateBalloon(L"Required", L"Port is a required field. Default is 27017", TTI_ERROR);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetDatabase());
  databaseLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Database*:", ChildId::DATABASE_LABEL);
  databaseEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::DATABASE_EDIT);
  databaseBalloon =
      CreateBalloon(L"Required", L"Database is a required field.", TTI_ERROR);
  rowPos += INTERVAL + ROW_HEIGHT;

  connectionSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Connection Settings",
                     ChildId::CONNECTION_SETTINGS_GROUP_BOX);

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateSshSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 120 };
  enum { BROWSE_BUTTON_WIDTH = 20 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  int checkBoxSize = sizeX - 2 * MARGIN;

  sshEnableCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Enable SSH Tunnel",
      ChildId::SSH_ENABLE_CHECK_BOX, config.IsSshEnable());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetSshUser());
  sshUserLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH User:", ChildId::SSH_USER_LABEL);
  sshUserEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::SSH_USER_EDIT);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshHost());
  sshHostLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"SSH Hostname:", ChildId::SSH_HOST_LABEL);
  sshHostEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::SSH_HOST_EDIT);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshPrivateKeyFile());
  sshPrivateKeyFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                       ROW_HEIGHT, L"SSH Private Key File:",
                                       ChildId::SSH_PRIVATE_KEY_FILE_LABEL);
  sshPrivateKeyFileEdit = CreateEdit(editPosX, rowPos, editSizeX - BROWSE_BUTTON_WIDTH, ROW_HEIGHT,
                                     wVal, ChildId::SSH_PRIVATE_KEY_FILE_EDIT);
  sshPrivateKeyFileBrowseButton = CreateButton(
      editPosX + editSizeX - BROWSE_BUTTON_WIDTH, rowPos, BROWSE_BUTTON_WIDTH, ROW_HEIGHT, L"...",
      ChildId::SSH_PRIV_KEY_FILE_BROWSE_BUTTON);
  rowPos += INTERVAL + ROW_HEIGHT;

  // SSH Strict Host Key Check check box needs to have editSizeX as size because
  // the string is long
  sshStrictHostKeyCheckingCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"SSH Strict Host Key Check (disabling option is less secure)",
      ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX,
      config.IsSshStrictHostKeyChecking());

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSshKnownHostsFile());
  sshKnownHostsFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                                       ROW_HEIGHT, L"SSH Known Hosts File:",
                                       ChildId::SSH_KNOWN_HOSTS_FILE_LABEL);
  sshKnownHostsFileEdit = CreateEdit(editPosX, rowPos, editSizeX - BROWSE_BUTTON_WIDTH, ROW_HEIGHT,
                                     wVal, ChildId::SSH_KNOWN_HOSTS_FILE_EDIT);
  sshKnownHostsFileBrowseButton = CreateButton(
      editPosX + editSizeX - BROWSE_BUTTON_WIDTH, rowPos, BROWSE_BUTTON_WIDTH,
      ROW_HEIGHT, L"...", ChildId::SSH_KNOW_HOSTS_FILE_BROWSE_BUTTON);

  rowPos += INTERVAL + ROW_HEIGHT;

  sshUserEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshPrivateKeyFileEdit->SetEnabled(sshEnableCheckBox->IsChecked());
  sshPrivateKeyFileBrowseButton->SetEnabled(sshEnableCheckBox->IsChecked());
  sshStrictHostKeyCheckingCheckBox->SetEnabled(sshEnableCheckBox->IsChecked());
  sshKnownHostsFileEdit->SetEnabled(
      sshEnableCheckBox->IsChecked()
      && sshStrictHostKeyCheckingCheckBox->IsChecked());
  sshKnownHostsFileBrowseButton->SetEnabled(
      sshEnableCheckBox->IsChecked()
      && sshStrictHostKeyCheckingCheckBox->IsChecked());

  sshGroupControls.push_back(sshEnableCheckBox.get());
  sshGroupControls.push_back(sshUserLabel.get());
  sshGroupControls.push_back(sshUserEdit.get());
  sshGroupControls.push_back(sshHostLabel.get());
  sshGroupControls.push_back(sshHostEdit.get());
  sshGroupControls.push_back(sshPrivateKeyFileLabel.get());
  sshGroupControls.push_back(sshPrivateKeyFileEdit.get());
  sshGroupControls.push_back(sshPrivateKeyFileBrowseButton.get());
  sshGroupControls.push_back(sshStrictHostKeyCheckingCheckBox.get());
  sshGroupControls.push_back(sshKnownHostsFileLabel.get());
  sshGroupControls.push_back(sshKnownHostsFileEdit.get());
  sshGroupControls.push_back(sshKnownHostsFileBrowseButton.get());
  tabGroupControls[TabIndex::Type::SSH_TUNNEL] = sshGroupControls;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateLogSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 100 };
  enum { BROWSE_BUTTON_WIDTH = 20 };

  int labelPosX = posX + INTERVAL;
  int pathSizeX = sizeX - 2 * INTERVAL - BROWSE_BUTTON_WIDTH;
  int comboSizeX = sizeX - LABEL_WIDTH - (3 * INTERVAL);
  int comboPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  LogLevel::Type logLevel = config.GetLogLevel();

  logLevelLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Log Level:", ChildId::LOG_LEVEL_LABEL);
  logLevelComboBox = CreateComboBox(comboPosX, rowPos, comboSizeX, ROW_HEIGHT,
                                    L"", ChildId::LOG_LEVEL_COMBO_BOX);

  int unknownIndex = LogLevel::ToInt(LogLevel::Type::UNKNOWN);
  for (auto i = 0; i < unknownIndex; i++) {
    logLevelComboBox->AddComboBoxItem(LogLevel::ToText(LogLevel::FromInt(i)));
  }

  logLevelComboBox->SetComboBoxSelection(
      LogLevel::ToInt(logLevel));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetLogPath());
  logPathLabel = CreateLabel(
      labelPosX, rowPos, pathSizeX, ROW_HEIGHT * 2,
      L"Log Path:\n(the log file name format is docdb_odbc_YYYYMMDD.log)",
      ChildId::LOG_PATH_LABEL);

  rowPos += INTERVAL * 2 + ROW_HEIGHT;

  logPathEdit = CreateEdit(labelPosX, rowPos, pathSizeX, ROW_HEIGHT, wVal,
                           ChildId::LOG_PATH_EDIT);
  int browseButtonPosX = labelPosX + pathSizeX;
  logPathBrowseButton =
      CreateButton(browseButtonPosX, rowPos, BROWSE_BUTTON_WIDTH, ROW_HEIGHT,
                   L"...", ChildId::LOG_PATH_BROWSE_BUTTON);

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring logLevelWStr;
  logLevelComboBox->GetText(logLevelWStr);
  if (LogLevel::FromString(utility::ToUtf8(logLevelWStr),
                           LogLevel::Type::UNKNOWN)
      == LogLevel::Type::OFF) {
    logPathEdit->SetEnabled(false);
    logPathBrowseButton->SetEnabled(false);
  } else {
    logPathEdit->SetEnabled(true);
    logPathBrowseButton->SetEnabled(true);
  }

  loggingGroupControls.push_back(logLevelLabel.get());
  loggingGroupControls.push_back(logLevelComboBox.get());
  loggingGroupControls.push_back(logPathLabel.get());
  loggingGroupControls.push_back(logPathEdit.get());
  loggingGroupControls.push_back(logPathBrowseButton.get());
  tabGroupControls[TabIndex::Type::LOGGING] = loggingGroupControls;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateTlsSettingsGroup(int posX, int posY,
                                                   int sizeX) {
  enum { LABEL_WIDTH = 100 };
  enum { BROWSE_BUTTON_WIDTH = 20 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  int checkBoxSize = sizeX - 2 * MARGIN;

  tlsCheckBox =
      CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Enable TLS",
                     ChildId::TLS_CHECK_BOX, config.IsTls());

  rowPos += INTERVAL + ROW_HEIGHT;

  tlsAllowInvalidHostnamesCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"Allow Invalid Hostnames (enabling option is less secure)",
      ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
      config.IsTlsAllowInvalidHostnames());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetTlsCaFile());
  tlsCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"TLS CA File:", ChildId::TLS_CA_FILE_LABEL);
  tlsCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX - BROWSE_BUTTON_WIDTH, ROW_HEIGHT, wVal,
                             ChildId::TLS_CA_FILE_EDIT);
  tlsCaFileBrowseButton = CreateButton(
      editPosX + editSizeX - BROWSE_BUTTON_WIDTH, rowPos, BROWSE_BUTTON_WIDTH,
      ROW_HEIGHT, L"...", ChildId::TLS_CA_FILE_BROWSE_BUTTON);

  rowPos += INTERVAL + ROW_HEIGHT;

  tlsAllowInvalidHostnamesCheckBox->SetEnabled(tlsCheckBox->IsChecked());
  tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());
  tlsCaFileBrowseButton->SetEnabled(tlsCheckBox->IsChecked());

  tlsGroupControls.push_back(tlsCheckBox.get());
  tlsGroupControls.push_back(tlsAllowInvalidHostnamesCheckBox.get());
  tlsGroupControls.push_back(tlsCaFileLabel.get());
  tlsGroupControls.push_back(tlsCaFileEdit.get());
  tlsGroupControls.push_back(tlsCaFileBrowseButton.get());
  tabGroupControls[TabIndex::Type::TLS] = tlsGroupControls;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateSchemaSettingsGroup(int posX, int posY,
                                                      int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY;

  int checkBoxSize = sizeX - 2 * MARGIN;

  ScanMethod::Type scanMethod = config.GetScanMethod();

  scanMethodLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                L"Scan Method:", ChildId::SCAN_METHOD_LABEL);
  scanMethodComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                                      L"", ChildId::SCAN_METHOD_COMBO_BOX);

  scanMethodComboBox->AddComboBoxItem(L"Random");
  scanMethodComboBox->AddComboBoxItem(L"ID Forward");
  scanMethodComboBox->AddComboBoxItem(L"ID Reverse");
  scanMethodComboBox->AddComboBoxItem(L"All");

  scanMethodComboBox->SetComboBoxSelection(
      static_cast< int >(scanMethod));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = std::to_wstring(config.GetScanLimit());
  scanLimitLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                               L"Scan Limit:", ChildId::SCAN_LIMIT_LABEL);
  scanLimitEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                             ChildId::SCAN_LIMIT_EDIT, ES_NUMBER);
  scanLimitBalloon = CreateBalloon(
      L"Required", L"Scan limit must be a positive numeric value. Default 1000 will be used instead.", TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetSchemaName());
  schemaLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                            L"Schema Name:", ChildId::SCHEMA_LABEL);
  schemaEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                          ChildId::SCHEMA_EDIT);
  schemaBalloon = CreateBalloon(
      L"Required",
      L"Schema name must not be blank. Default '_default' will be used instead.",
      TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  refreshSchemaCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
      L"Refresh Schema (Caution: use temporarily to update schema)",
      ChildId::REFRESH_SCHEMA_CHECK_BOX, config.IsRefreshSchema());

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring scanMethodWStr;
  scanMethodComboBox->GetText(scanMethodWStr);
  if (ScanMethod::FromString(utility::ToUtf8(scanMethodWStr),
                             ScanMethod::Type::UNKNOWN)
      == ScanMethod::Type::ALL) {
    scanLimitEdit->SetEnabled(false);
  } else {
    scanLimitEdit->SetEnabled(true);
  }

  schemaGroupControls.push_back(scanMethodLabel.get());
  schemaGroupControls.push_back(scanMethodComboBox.get());
  schemaGroupControls.push_back(scanLimitLabel.get());
  schemaGroupControls.push_back(scanLimitEdit.get());
  schemaGroupControls.push_back(schemaLabel.get());
  schemaGroupControls.push_back(schemaEdit.get());
  schemaGroupControls.push_back(refreshSchemaCheckBox.get());
  tabGroupControls[TabIndex::Type::SCHEMA] = schemaGroupControls;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateAdditionalSettingsGroup(int posX, int posY,
                                                          int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int checkBoxSize = (sizeX - 3 * INTERVAL) / 2;

  int rowPos = posY;

  retryReadsCheckBox = CreateCheckBox(
      labelPosX, rowPos, checkBoxSize, ROW_HEIGHT, L"Retry Reads",
      ChildId::RETRY_READS_CHECK_BOX, config.IsRetryReads());

  rowPos += INTERVAL + ROW_HEIGHT;

  ReadPreference::Type readPreference = config.GetReadPreference();

  readPreferenceLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Read preference:", ChildId::READ_PREFERENCE_LABEL);
  readPreferenceComboBox =
      CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT, L"",
                     ChildId::READ_PREFERENCE_COMBO_BOX);

  readPreferenceComboBox->AddComboBoxItem(L"Primary");
  readPreferenceComboBox->AddComboBoxItem(L"Primary Preferred");
  readPreferenceComboBox->AddComboBoxItem(L"Secondary");
  readPreferenceComboBox->AddComboBoxItem(L"Secondary Preferred");
  readPreferenceComboBox->AddComboBoxItem(L"Nearest");

  readPreferenceComboBox->SetComboBoxSelection(
      static_cast< int >(readPreference));  // set default

  rowPos += INTERVAL + ROW_HEIGHT;

  std::wstring wVal = utility::FromUtf8(config.GetApplicationName());
  appNameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                             L"Application Name:", ChildId::APP_NAME_LABEL);
  appNameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                           ChildId::APP_NAME_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetLoginTimeoutSeconds());
  loginTimeoutSecLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                  L"Login Timeout (s):", ChildId::LOGIN_TIMEOUT_SEC_LABEL);

  loginTimeoutSecEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                 ChildId::LOGIN_TIMEOUT_SEC_EDIT, ES_NUMBER);
  loginTimeoutSecBalloon =
      CreateBalloon(L"Required",
                    L"Login timeout must be a positive numeric value. Default "
                    L"0 will be used instead.",
                    TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = utility::FromUtf8(config.GetReplicaSet());
  replicaSetLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                L"Replica Set:", ChildId::REPLICA_SET_LABEL);
  replicaSetEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                              ChildId::REPLICA_SET_EDIT);

  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = std::to_wstring(config.GetDefaultFetchSize());
  defaultFetchSizeLabel =
      CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"Fetch Size:",
                  ChildId::DEFAULT_FETCH_SIZE_LABEL);

  defaultFetchSizeEdit =
      CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                 ChildId::DEFAULT_FETCH_SIZE_EDIT, ES_NUMBER);
  defaultFetchSizeBalloon =
      CreateBalloon(L"Required",
                    L"Fetch size must be a positive numeric value. Default "
                    L"2000 will be used instead.",
                    TTI_ERROR);

  rowPos += INTERVAL + ROW_HEIGHT;

  additionalGroupControls.push_back(retryReadsCheckBox.get());
  additionalGroupControls.push_back(readPreferenceLabel.get());
  additionalGroupControls.push_back(readPreferenceComboBox.get());
  additionalGroupControls.push_back(appNameLabel.get());
  additionalGroupControls.push_back(appNameEdit.get());
  additionalGroupControls.push_back(loginTimeoutSecLabel.get());
  additionalGroupControls.push_back(loginTimeoutSecEdit.get());
  additionalGroupControls.push_back(replicaSetLabel.get());
  additionalGroupControls.push_back(replicaSetEdit.get());
  additionalGroupControls.push_back(defaultFetchSizeLabel.get());
  additionalGroupControls.push_back(defaultFetchSizeEdit.get());
  tabGroupControls[TabIndex::Type::ADDITIONAL] = additionalGroupControls;

  return rowPos - posY;
}

int DsnConfigurationWindow::CreateTestSettingsGroup(int posX, int posY,
                                                    int sizeX) {
  enum { LABEL_WIDTH = 100 };

  int labelPosX = posX + INTERVAL;

  int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
  int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

  int rowPos = posY + 2 * INTERVAL;

  std::wstring wVal;

  // Ignore any existing setting in the DSN for user/password.
  wVal = L"";
  userLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, L"User :",
                          ChildId::USER_LABEL);
  userEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                        ChildId::USER_EDIT);
  rowPos += INTERVAL + ROW_HEIGHT;

  wVal = L"";
  passwordLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                              L"Password:", ChildId::PASSWORD_LABEL);
  passwordEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, wVal,
                            ChildId::USER_EDIT, ES_PASSWORD);
  rowPos += INTERVAL + ROW_HEIGHT;

  int testPosX = (width - (MARGIN * 2)) - BUTTON_WIDTH;
  int testLabelSizeX = testPosX - labelPosX - INTERVAL;
  testLabel = CreateLabel(labelPosX, rowPos, testLabelSizeX, BUTTON_HEIGHT,
                          L"Enter valid User and Password to test "
                          L"the connection settings.",
                          ChildId::TEST_LABEL);
  
  testButton = CreateButton(testPosX, rowPos, BUTTON_WIDTH, BUTTON_HEIGHT,
                            L"Test", ChildId::TEST_BUTTON);
  rowPos += INTERVAL + ROW_HEIGHT;

  testSettingsGroupBox =
      CreateGroupBox(posX, posY, sizeX, rowPos - posY, L"Test Connection",
                     ChildId::CONNECTION_SETTINGS_GROUP_BOX);

  return rowPos - posY;
}

void DsnConfigurationWindow::TestConnection() const {
  // Use a temporary configuration to test the connection.
  // Don't want to commit the changes until OK button is pressed.
  config::Configuration tempConfig;
  RetrieveParameters(tempConfig);
  RetrieveTestParameters(tempConfig);
  std::vector< SQLWCHAR > vDsn =
      utility::ToWCHARVector(tempConfig.ToConnectString());

  // Allocate an environment handle
  SQLHENV env = {};
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to allocate Environment handle.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    goto CLEANUP_ENVIRONMENT;
  }

  // We want ODBC 3 support
  ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                      reinterpret_cast< void* >(SQL_OV_ODBC3), 0);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to set ODBC version.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    goto CLEANUP_ENVIRONMENT;
  }

  // Allocate a connection handle
  SQLHDBC dbc = {};
  ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
  if (!SQL_SUCCEEDED(ret)) {
    MessageBox(handle, L"Unable to allocate Connection handle.", L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    goto CLEANUP_ENVIRONMENT;
  }

  // Test the connection.
  ret = SQLDriverConnect(dbc, nullptr, vDsn.data(), vDsn.size(), nullptr, 0,
                         nullptr, SQL_DRIVER_COMPLETE);
  if (!SQL_SUCCEEDED(ret)) {
    SQLWCHAR sqlState[7];
    SQLINTEGER nativeCode;
    SQLWCHAR errMessage[1024];
    SQLGetDiagRec(SQL_HANDLE_DBC, dbc, 1, sqlState, &nativeCode, errMessage,
                  sizeof(errMessage) / sizeof(SQLWCHAR), nullptr);
    std::stringstream buf;
    buf << "Connection failed: '" << utility::SqlWcharToString(errMessage)
        << "'";
    std::vector< SQLWCHAR > vErrMessage = utility::ToWCHARVector(buf.str());
    MessageBox(handle, vErrMessage.data(), L"Error!",
               MB_ICONEXCLAMATION | MB_OK);
    goto CLEANUP_CONNECTION;
  }

  MessageBox(handle, L"Connection succeeded.", L"Success!",
             MB_ICONINFORMATION | MB_OK);

  // Cleanup
  SQLDisconnect(dbc);
CLEANUP_CONNECTION:
  SQLFreeHandle(SQL_HANDLE_DBC, dbc);
CLEANUP_ENVIRONMENT:
  SQLFreeHandle(SQL_HANDLE_ENV, env);
}

// Callback function to set the initial path.
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,
                                LPARAM lpData) {
  switch (uMsg) {
    case BFFM_INITIALIZED: {
      if (lpData != NULL)
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
    } break;
  }
  return 0;
}

bool DsnConfigurationWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case ChildId::OK_BUTTON: {
          try {
            RetrieveParameters(config);

            accepted = true;

            PostMessage(GetHandle(), WM_CLOSE, 0, 0);
          } catch (DocumentDbError& err) {
            std::wstring errWText = utility::FromUtf8(err.GetText());
            MessageBox(NULL, errWText.c_str(), L"Error!",
                       MB_ICONEXCLAMATION | MB_OK);
          }

          break;
        }

        case IDCANCEL:
        case ChildId::CANCEL_BUTTON: {
          PostMessage(GetHandle(), WM_CLOSE, 0, 0);

          break;
        }

        case ChildId::TEST_BUTTON: {
          TestConnection();
          break;
        }

        case ChildId::NAME_EDIT: {
          if (created) {
            if (!shownNameBalloon && !nameEdit->HasText()) {
              Edit_ShowBalloonTip(nameEdit->GetHandle(), nameBalloon.get());
              shownNameBalloon = true;
            } else {
              Edit_HideBalloonTip(nameEdit->GetHandle());
              shownNameBalloon = false;
            }
            saveButton->SetEnabled(VALIDATE_FOR_SAVE());
            testButton->SetEnabled(VALIDATE_FOR_TEST());
          }
          break;
        }

        case ChildId::HOST_NAME_EDIT: {
          if (created) {
            if (!shownHostnameBalloon && !hostnameEdit->HasText()) {
              Edit_ShowBalloonTip(hostnameEdit->GetHandle(),
                                  hostnameBalloon.get());
              shownHostnameBalloon = true;
            } else {
              Edit_HideBalloonTip(hostnameEdit->GetHandle());
              shownHostnameBalloon = false;
            }
            saveButton->SetEnabled(VALIDATE_FOR_SAVE());
            testButton->SetEnabled(VALIDATE_FOR_TEST());
          }
          break;
        }

        case ChildId::PORT_EDIT: {
          if (created) {
            if (!shownPortBalloon && !portEdit->HasText()) {
              Edit_ShowBalloonTip(portEdit->GetHandle(), portBalloon.get());
              shownPortBalloon = true;
            } else {
              Edit_HideBalloonTip(portEdit->GetHandle());
              shownPortBalloon = false;
            }
            saveButton->SetEnabled(VALIDATE_FOR_SAVE());
            testButton->SetEnabled(VALIDATE_FOR_TEST());
          }
          break;
        }

        case ChildId::DATABASE_EDIT: {
          if (created) {
            if (!shownDatabaseBalloon && !databaseEdit->HasText()) {
              Edit_ShowBalloonTip(
                  databaseEdit->GetHandle(),
                  databaseBalloon.get());
              shownDatabaseBalloon = true;
            } else {
              Edit_HideBalloonTip(databaseEdit->GetHandle());
              shownDatabaseBalloon = false;
            }
            saveButton->SetEnabled(VALIDATE_FOR_SAVE());
            testButton->SetEnabled(VALIDATE_FOR_TEST());
          }
          break;
        }

        case ChildId::USER_EDIT:
        case ChildId::PASSWORD_EDIT: {
          if (created) {
            saveButton->SetEnabled(VALIDATE_FOR_SAVE());
            testButton->SetEnabled(VALIDATE_FOR_TEST());
          }
          break;
        }

        case ChildId::SSH_ENABLE_CHECK_BOX: {
          sshEnableCheckBox->SetChecked(!sshEnableCheckBox->IsChecked());
          sshUserEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshHostEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshPrivateKeyFileEdit->SetEnabled(sshEnableCheckBox->IsChecked());
          sshPrivateKeyFileBrowseButton->SetEnabled(
              sshEnableCheckBox->IsChecked());
          sshStrictHostKeyCheckingCheckBox->SetEnabled(
              sshEnableCheckBox->IsChecked());
          sshKnownHostsFileEdit->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());
          sshKnownHostsFileBrowseButton->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());

          break;
        }

        case ChildId::SSH_STRICT_HOST_KEY_CHECKING_CHECK_BOX: {
          sshStrictHostKeyCheckingCheckBox->SetChecked(
              !sshStrictHostKeyCheckingCheckBox->IsChecked());
          sshKnownHostsFileEdit->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());
          sshKnownHostsFileBrowseButton->SetEnabled(
              sshEnableCheckBox->IsChecked()
              && sshStrictHostKeyCheckingCheckBox->IsChecked());
          break;
        }

        case ChildId::TLS_CHECK_BOX: {
          tlsCheckBox->SetChecked(!tlsCheckBox->IsChecked());
          tlsAllowInvalidHostnamesCheckBox->SetEnabled(
              tlsCheckBox->IsChecked());
          tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());
          tlsCaFileBrowseButton->SetEnabled(tlsCheckBox->IsChecked());

          break;
        }

        case ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX: {
          tlsAllowInvalidHostnamesCheckBox->SetChecked(
              !tlsAllowInvalidHostnamesCheckBox->IsChecked());

          break;
        }

        case ChildId::LOG_LEVEL_COMBO_BOX: {
          std::wstring logLevelWStr;
          logLevelComboBox->GetText(logLevelWStr);
          if (LogLevel::FromString(utility::ToUtf8(logLevelWStr),
                                   LogLevel::Type::UNKNOWN)
              == LogLevel::Type::OFF) {
            logPathEdit->SetEnabled(false);
            logPathBrowseButton->SetEnabled(false);
          } else {
            logPathEdit->SetEnabled(true);
            logPathBrowseButton->SetEnabled(true);
          }
          break;
        }

        case ChildId::SCAN_METHOD_COMBO_BOX: {
          std::wstring scanMethodWStr;
          scanMethodComboBox->GetText(scanMethodWStr);
          if (ScanMethod::FromString(utility::ToUtf8(scanMethodWStr),
                                     ScanMethod::Type::UNKNOWN)
              == ScanMethod::Type::ALL) {
            scanLimitEdit->SetEnabled(false);
          } else {
            scanLimitEdit->SetEnabled(true);
          }
          break;
        }

        case ChildId::SCAN_LIMIT_EDIT: {
          if (created) {
            long scanLimit;
            std::wstring wScanLimit;
            wchar_t* pEnd;
            scanLimitEdit->GetText(wScanLimit);
            if (!shownScanLimitBalloon
                && (!scanLimitEdit->HasText()
                    || (scanLimit = std::wcstol(wScanLimit.c_str(), &pEnd, 10))
                           <= 0)) {
              Edit_ShowBalloonTip(scanLimitEdit->GetHandle(),
                                  scanLimitBalloon.get());
              shownScanLimitBalloon = true;
            } else {
              Edit_HideBalloonTip(scanLimitEdit->GetHandle());
              shownScanLimitBalloon = false;
            }
          }
          break;
        }

        case ChildId::SCHEMA_EDIT: {
          if (created) {
            if (!shownSchemaBalloon && !schemaEdit->HasText()) {
              Edit_ShowBalloonTip(schemaEdit->GetHandle(),
                                  schemaBalloon.get());
              shownSchemaBalloon = true;
            } else {
              Edit_HideBalloonTip(schemaEdit->GetHandle());
              shownSchemaBalloon = false;
            }
          }
          break;
        }

        case ChildId::LOGIN_TIMEOUT_SEC_EDIT: {
          if (created) {
            if (!shownLoginTimeoutSecBalloon
                && !loginTimeoutSecEdit->HasText()) {
              Edit_ShowBalloonTip(loginTimeoutSecEdit->GetHandle(),
                                  loginTimeoutSecBalloon.get());
              shownLoginTimeoutSecBalloon = true;
            } else {
              Edit_HideBalloonTip(loginTimeoutSecEdit->GetHandle());
              shownLoginTimeoutSecBalloon = false;
            }
          }
          break;
        }

        case ChildId::DEFAULT_FETCH_SIZE_EDIT: {
          if (created) {
            long defaultFetchSize;
            std::wstring wDefaultFetchSize;
            wchar_t* pEnd;
            defaultFetchSizeEdit->GetText(wDefaultFetchSize);
            if (!shownDefaultFetchSizeBalloon
                && (!defaultFetchSizeEdit->HasText()
                    || (defaultFetchSize =
                            std::wcstol(wDefaultFetchSize.c_str(), &pEnd, 10))
                           <= 0)) {
              Edit_ShowBalloonTip(defaultFetchSizeEdit->GetHandle(),
                                  defaultFetchSizeBalloon.get());
              shownDefaultFetchSizeBalloon = true;
            } else {
              Edit_HideBalloonTip(defaultFetchSizeEdit->GetHandle());
              shownDefaultFetchSizeBalloon = false;
            }
          }
          break;
        }

        case ChildId::REFRESH_SCHEMA_CHECK_BOX: {
          refreshSchemaCheckBox->SetChecked(
              !refreshSchemaCheckBox->IsChecked());
          break;
        }

        case ChildId::RETRY_READS_CHECK_BOX: {
          retryReadsCheckBox->SetChecked(!retryReadsCheckBox->IsChecked());

          break;
        }

        case ChildId::LOG_PATH_BROWSE_BUTTON: {
          std::wstring initLogPath;
          logPathEdit->GetText(initLogPath);
          std::unique_ptr< BROWSEINFO > bi(std::make_unique< BROWSEINFO >());
          bi->lpszTitle = L"Choose log file target directory:";
          bi->ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
          bi->hwndOwner = logPathBrowseButton->GetHandle();
          bi->lpfn = BrowseCallbackProc;
          bi->lParam = reinterpret_cast< LPARAM >(initLogPath.c_str());

          const LPITEMIDLIST& pidl = SHBrowseForFolder(bi.get());

          if (pidl != nullptr) {
            // get the name of the folder and put it in the log path field
            wchar_t logPath[_MAX_PATH];
            SHGetPathFromIDList(pidl, logPath);
            logPathEdit->SetText(static_cast< std::wstring >(logPath));
          }

          break;
        }

        case ChildId::SSH_PRIV_KEY_FILE_BROWSE_BUTTON: {
          std::wstring initPathStr;
          sshPrivateKeyFileEdit->GetText(initPathStr);
          std::vector< wchar_t > initPath(initPathStr.begin(), initPathStr.end());
          initPath.resize((MAX_PATH * 2) + 1);

          if (GetFileNameFromBrowse(sshPrivateKeyFileBrowseButton->GetHandle(),
                                    initPath.data(), initPath.size(), nullptr,
                                    L"pem", nullptr,
                                    L"Choose SSH private key file.")) {
            sshPrivateKeyFileEdit->SetText(initPath.data());
          }

          break;
        }

        case ChildId::SSH_KNOW_HOSTS_FILE_BROWSE_BUTTON: {
          std::wstring initPathStr;
          sshKnownHostsFileEdit->GetText(initPathStr);
          std::vector< wchar_t > initPath(initPathStr.begin(),
                                          initPathStr.end());
          initPath.resize((MAX_PATH * 2) + 1);

          if (GetFileNameFromBrowse(sshKnownHostsFileBrowseButton->GetHandle(),
                                    initPath.data(), initPath.size(), nullptr,
                                    L"", nullptr,
                                    L"Choose SSH known hosts file.")) {
            sshKnownHostsFileEdit->SetText(initPath.data());
          }

          break;
        }

        case ChildId::TLS_CA_FILE_BROWSE_BUTTON: {
          std::wstring initPathStr;
          tlsCaFileEdit->GetText(initPathStr);
          std::vector< wchar_t > initPath(initPathStr.begin(),
                                          initPathStr.end());
          initPath.resize((MAX_PATH * 2) + 1);

          if (GetFileNameFromBrowse(tlsCaFileBrowseButton->GetHandle(),
                                    initPath.data(), initPath.size(), nullptr,
                                    L"pem", nullptr,
                                    L"Choose AWS Certficate Authority (CA) file.")) {
            tlsCaFileEdit->SetText(initPath.data());
          }

          break;
        }

        default:
          return false;
      }

      break;
    }

    case WM_DESTROY: {
      PostQuitMessage(accepted ? Result::OK : Result::CANCEL);

      break;
    }

    case WM_NOTIFY: {
      switch (LOWORD(wParam)) {
        case ChildId::TABS: {
          LOG_DEBUG_MSG("current Tab selection index (without cast): "
                        << tabs->GetTabSelection());

          TabIndex::Type curSel =
              static_cast< TabIndex::Type >(tabs->GetTabSelection());

          LOG_DEBUG_MSG("current Tab selection index (with cast): " << curSel);

          OnSelectedTabChange(curSel);
          break;
        }

        default:
          return false;
      }

      break;
    }

    default:
      return false;
  }

  return true;
}

void DsnConfigurationWindow::SetTabGroupVisible(TabIndex::Type idx,
                                                bool isVisible) {
  for (auto control : tabGroupControls[idx]) {
    control->Show(isVisible);
  }
}

void DsnConfigurationWindow::OnSelectedTabChange(TabIndex::Type idx) {
  if (idx == prevSelectedTabIndex)
    return;
  SetTabGroupVisible(prevSelectedTabIndex, false);
  SetTabGroupVisible(idx);
  prevSelectedTabIndex = idx;
}

void DsnConfigurationWindow::RetrieveParameters(
    config::Configuration& cfg) const {
  RetrieveLogParameters(cfg);
  RetrieveConnectionParameters(cfg);
  RetrieveSshParameters(cfg);
  RetrieveTlsParameters(cfg);
  RetrieveSchemaParameters(cfg);
  RetrieveAdditionalParameters(cfg);
}

void DsnConfigurationWindow::RetrieveConnectionParameters(
    config::Configuration& cfg) const {
  std::wstring dsnWStr;
  std::wstring hostnameWStr;
  std::wstring portWStr;
  std::wstring databaseWStr;

  nameEdit->GetText(dsnWStr);
  std::string dsnStr = utility::ToUtf8(dsnWStr);
  common::StripSurroundingWhitespaces(dsnStr);
  // Stripping of whitespaces off the schema skipped intentionally

  hostnameEdit->GetText(hostnameWStr);
  portEdit->GetText(portWStr);
  databaseEdit->GetText(databaseWStr);

  std::string hostnameStr = utility::ToUtf8(hostnameWStr);
  std::string portStr = utility::ToUtf8(portWStr);
  std::string databaseStr = utility::ToUtf8(databaseWStr);

  int16_t port = common::LexicalCast< int16_t >(portStr);

  // If invalid values, use default instead.
  if (port <= 0)
    port = Configuration::DefaultValue::port;

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("DSN:      " << dsnStr);
  LOG_MSG("Hostname: " << hostnameStr);
  LOG_MSG("Port:     " << portStr);
  LOG_MSG("Database: " << databaseStr);

  cfg.SetDsn(dsnStr);
  cfg.SetPort(port);
  cfg.SetHostname(hostnameStr);
  cfg.SetDatabase(databaseStr);
}

void DsnConfigurationWindow::RetrieveSshParameters(
    config::Configuration& cfg) const {
  bool sshEnable = sshEnableCheckBox->IsChecked();
  bool sshStrictHostKeyChecking = sshStrictHostKeyCheckingCheckBox->IsChecked();

  std::wstring sshUserWStr;
  std::wstring sshHostWStr;
  std::wstring sshPrivateKeyFileWStr;
  std::wstring sshPrivateKeyPassphraseWStr;
  std::wstring sshKnownHostsFileWStr;

  sshUserEdit->GetText(sshUserWStr);
  sshHostEdit->GetText(sshHostWStr);
  sshPrivateKeyFileEdit->GetText(sshPrivateKeyFileWStr);
  sshKnownHostsFileEdit->GetText(sshKnownHostsFileWStr);

  std::string sshUserStr = utility::ToUtf8(sshUserWStr);
  std::string sshHostStr = utility::ToUtf8(sshHostWStr);
  std::string sshPrivateKeyFileStr = utility::ToUtf8(sshPrivateKeyFileWStr);
  std::string sshPrivateKeyPassphraseStr =
      utility::ToUtf8(sshPrivateKeyPassphraseWStr);
  std::string sshKnownHostsFileStr = utility::ToUtf8(sshKnownHostsFileWStr);

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("SSH enable:                    " << (sshEnable ? "true" : "false"));
  LOG_MSG("SSH user:                      " << sshUserStr);
  LOG_MSG("SSH host:                      " << sshHostStr);
  LOG_MSG("SSH private key file:          " << sshPrivateKeyFileStr);
  LOG_MSG("SSH strict host key checking:  "
          << (sshStrictHostKeyChecking ? "true" : "false"));
  LOG_MSG("SSH known hosts file:          " << sshKnownHostsFileStr);

  cfg.SetSshEnable(sshEnable);
  cfg.SetSshUser(sshUserStr);
  cfg.SetSshHost(sshHostStr);
  cfg.SetSshPrivateKeyFile(sshPrivateKeyFileStr);
  cfg.SetSshPrivateKeyPassphrase(sshPrivateKeyPassphraseStr);
  cfg.SetSshStrictHostKeyChecking(sshStrictHostKeyChecking);
  cfg.SetSshKnownHostsFile(sshKnownHostsFileStr);
}

// RetrieveLogParameters is a special case. We want to get the log level and
// path as soon as possible. If user set log level to OFF, then nothing should
// be logged. Therefore, the LOG_MSG calls are after log level and log path are
// set.
void DsnConfigurationWindow::RetrieveLogParameters(
    config::Configuration& cfg) const {
  std::wstring logLevelWStr;
  std::wstring logPathWStr;

  logLevelComboBox->GetText(logLevelWStr);
  logPathEdit->GetText(logPathWStr);

  std::string logLevelStr = utility::ToUtf8(logLevelWStr);
  std::string logPathStr = utility::ToUtf8(logPathWStr);

  LogLevel::Type logLevel =
      LogLevel::FromString(logLevelStr, LogLevel::Type::UNKNOWN);

  cfg.SetLogLevel(logLevel);
  cfg.SetLogPath(logPathStr);

  LOG_MSG("Log level:    " << logLevelStr);
  LOG_MSG("Log path:     " << logPathStr);
}

void DsnConfigurationWindow::RetrieveTlsParameters(
    config::Configuration& cfg) const {
  bool tls = tlsCheckBox->IsChecked();
  bool tlsAllowInvalidHostnames = tlsAllowInvalidHostnamesCheckBox->IsChecked();
  std::wstring tlsCaWStr;

  tlsCaFileEdit->GetText(tlsCaWStr);

  std::string tlsCaStr = utility::ToUtf8(tlsCaWStr);

  LOG_MSG(
      "TLS/SSL Encryption:                       " << (tls ? "true" : "false"));
  LOG_MSG("TLS Allow Invalid Hostnames:              "
          << (tlsAllowInvalidHostnames ? "true" : "false"));
  LOG_MSG("TLS CA (Certificate Authority) File: " << tlsCaStr);

  cfg.SetTls(tls);
  cfg.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames);
  cfg.SetTlsCaFile(tlsCaStr);
}

void DsnConfigurationWindow::RetrieveSchemaParameters(
    config::Configuration& cfg) const {
  std::wstring scanMethodWStr;
  std::wstring scanLimitWStr;
  std::wstring schemaWStr;
  bool refreshSchema = refreshSchemaCheckBox->IsChecked();

  scanMethodComboBox->GetText(scanMethodWStr);
  scanLimitEdit->GetText(scanLimitWStr);
  schemaEdit->GetText(schemaWStr);

  std::string scanMethodStr = utility::ToUtf8(scanMethodWStr);
  std::string scanLimitStr = utility::ToUtf8(scanLimitWStr);
  std::string schemaStr = utility::ToUtf8(schemaWStr);

  int32_t scanLimit = common::LexicalCast< int32_t >(scanLimitStr);
  // If invalid values, use default instead.
  if (scanLimit <= 0)
    scanLimit = Configuration::DefaultValue::scanLimit;
  if (schemaStr.empty())
    schemaStr = Configuration::DefaultValue::schemaName;

  LOG_MSG("Scan method:    " << scanMethodStr);
  LOG_MSG("Scan limit      " << scanLimit);
  LOG_MSG("Schema:         " << schemaStr);
  LOG_MSG("Refresh schema: " << (refreshSchema ? "true" : "false"));

  ScanMethod::Type scanMethod =
      ScanMethod::FromString(scanMethodStr, ScanMethod::Type::UNKNOWN);

  cfg.SetScanMethod(scanMethod);
  cfg.SetSchemaName(schemaStr);
  cfg.SetScanLimit(scanLimit);
  cfg.SetRefreshSchema(refreshSchema);
}

void DsnConfigurationWindow::RetrieveAdditionalParameters(
    config::Configuration& cfg) const {
  std::wstring readPreferenceWStr;
  std::wstring appNameWStr;
  std::wstring replicaSetWStr;
  std::wstring loginTimeoutSecWStr;
  std::wstring fetchSizeWStr;

  readPreferenceComboBox->GetText(readPreferenceWStr);
  appNameEdit->GetText(appNameWStr);
  replicaSetEdit->GetText(replicaSetWStr);
  bool retryReads = retryReadsCheckBox->IsChecked();
  loginTimeoutSecEdit->GetText(loginTimeoutSecWStr);
  defaultFetchSizeEdit->GetText(fetchSizeWStr);

  std::string readPreferenceStr = utility::ToUtf8(readPreferenceWStr);
  std::string appNameStr = utility::ToUtf8(appNameWStr);
  std::string replicaSetStr = utility::ToUtf8(replicaSetWStr);
  std::string loginTimeoutSecStr = utility::ToUtf8(loginTimeoutSecWStr);
  std::string fetchSizeStr = utility::ToUtf8(fetchSizeWStr);

  int32_t loginTimeoutSec = common::LexicalCast< int32_t >(loginTimeoutSecStr);
  // Note: zero indicates no limit on timeout.
  // If invalid values, use default instead.
  if (loginTimeoutSec < 0)
    loginTimeoutSec = Configuration::DefaultValue::loginTimeoutSec;

  int32_t fetchSize = common::LexicalCast< int32_t >(fetchSizeStr);
  // If invalid values, use default instead.
  if (fetchSize <= 0)
    fetchSize = Configuration::DefaultValue::defaultFetchSize;

  LOG_MSG("Retrieving arguments:");
  LOG_MSG("Retry reads:              " << (retryReads ? "true" : "false"));
  LOG_MSG("Read preference:          " << readPreferenceStr);
  LOG_MSG("App name:                 " << appNameStr);
  LOG_MSG("Login timeout (seconds):  " << loginTimeoutSecStr);
  LOG_MSG("Replica Set:              " << replicaSetStr);
  LOG_MSG("Fetch size:               " << fetchSize);

  ReadPreference::Type readPreference = ReadPreference::FromString(
      readPreferenceStr, ReadPreference::Type::UNKNOWN);

  cfg.SetReadPreference(readPreference);
  cfg.SetRetryReads(retryReads);
  cfg.SetApplicationName(appNameStr);
  cfg.SetLoginTimeoutSeconds(loginTimeoutSec);
  cfg.SetReplicaSet(replicaSetStr);
  cfg.SetDefaultFetchSize(fetchSize);
}

void DsnConfigurationWindow::RetrieveTestParameters(config::Configuration& cfg) const {
  std::wstring userWStr;
  std::wstring passwordWStr;

  userEdit->GetText(userWStr);
  passwordEdit->GetText(passwordWStr);

  std::string userStr = utility::ToUtf8(userWStr);
  std::string passwordStr = utility::ToUtf8(passwordWStr);

  // username and password intentionally not logged for security reasons

  cfg.SetUser(userStr);
  cfg.SetPassword(passwordStr);
}
}  // namespace ui
}  // namespace system
}  // namespace odbc
}  // namespace documentdb
