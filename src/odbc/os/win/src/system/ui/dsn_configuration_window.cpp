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

#include <Windowsx.h>
#include <Shlwapi.h>

#include "ignite/odbc/log.h"
#include "ignite/odbc/ssl_mode.h"

#include "ignite/odbc/system/ui/dsn_configuration_window.h"
#include "ignite/odbc/config/config_tools.h"
#include "ignite/odbc/diagnostic/diagnosable_adapter.h"

namespace ignite
{
    namespace odbc
    {
        namespace system
        {
            namespace ui
            {   // -AL- the constructor. No-op means no operation I think? 
                DsnConfigurationWindow::DsnConfigurationWindow(Window* parent, config::Configuration& config):
                    CustomWindow(parent, "IgniteConfigureDsn", "Configure Amazon DocumentDB DSN Latest"),
                    width(360),
                    height(600),
                    connectionSettingsGroupBox(),
                    sslSettingsGroupBox(), // has a create... function defined 
                    tlsCheckBox(), 
                    authSettingsGroupBox(),
                    additionalSettingsGroupBox(),
                    nameLabel(),
                    nameEdit(),
                    addressLabel(),
                    addressEdit(),
                    schemaLabel(),
                    schemaEdit(),
                    appNameLabel(),
                    appNameEdit(),
                    readPreferenceLabel(),
                    readPreferenceEdit(),
                    fetchSizeLabel(),
                    fetchSizeEdit(),
                    // pageSizeLabel(),
                    // pageSizeEdit(),
                    //distributedJoinsCheckBox(),
                    //enforceJoinOrderCheckBox(),
                    //replicatedOnlyCheckBox(),
                    //collocatedCheckBox(), // -AL- after commenting this out, the checkbox is intact?
                    // checkbox is created in function DsnConfigurationWindow::CreateAdditionalSettingsGroup
                    protocolVersionLabel(),
                    protocolVersionComboBox(),
                    userLabel(),
                    userEdit(),
                    passwordLabel(),
                    passwordEdit(),
                    //nestedTxModeComboBox(),
                    okButton(),
                    cancelButton(),
                    config(config),
                    accepted(false)
                {
                    // No-op.
                }

                DsnConfigurationWindow::~DsnConfigurationWindow()
                {
                    // No-op.
                }

                void DsnConfigurationWindow::Create()
                {
                    // Finding out parent position.
                    RECT parentRect;
                    GetWindowRect(parent->GetHandle(), &parentRect);

                    // Positioning window to the center of parent window.
                    const int posX = parentRect.left + (parentRect.right - parentRect.left - width) / 2;
                    const int posY = parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;

                    RECT desiredRect = {posX, posY, posX + width, posY + height};
                    AdjustWindowRect(&desiredRect, WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);

                    Window::Create(WS_OVERLAPPED | WS_SYSMENU, desiredRect.left, desiredRect.top,
                        desiredRect.right - desiredRect.left, desiredRect.bottom - desiredRect.top, 0);

                    if (!handle)
                    {
                        std::stringstream buf;

                        buf << "Can not create window, error code: " << GetLastError();

                        throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
                    }
                }
                // the function that actually creates the UI -AL-
                void DsnConfigurationWindow::OnCreate()
                {
                    int groupPosY = MARGIN;
                    int groupSizeY = width - 2 * MARGIN;

                    groupPosY += INTERVAL + CreateConnectionSettingsGroup(MARGIN, groupPosY, groupSizeY);
                    groupPosY += INTERVAL + CreateAuthSettingsGroup(MARGIN, groupPosY, groupSizeY);
                    groupPosY += INTERVAL + CreateSslSettingsGroup(MARGIN, groupPosY, groupSizeY);
                    groupPosY += INTERVAL + CreateAdditionalSettingsGroup(MARGIN, groupPosY, groupSizeY);
                    // test: if above code is commented out, additional settings shouldn't appear in config window. Result: Yes test success. 
                    // what happens here is the height of each subgroup is calculated and appended to the y position of the buttons

                    int cancelPosX = width - MARGIN - BUTTON_WIDTH;
                    int okPosX = cancelPosX - INTERVAL - BUTTON_WIDTH;

                    okButton = CreateButton(okPosX, groupPosY, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok", ChildId::OK_BUTTON);
                    cancelButton = CreateButton(cancelPosX, groupPosY, BUTTON_WIDTH, BUTTON_HEIGHT,
                        "Cancel", ChildId::CANCEL_BUTTON);
                }

                int DsnConfigurationWindow::CreateConnectionSettingsGroup(int posX, int posY, int sizeX)
                {
                    enum { LABEL_WIDTH = 100 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    const char* val = config.GetDsn().c_str();
                    nameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Data Source Name:", ChildId::NAME_LABEL);
                    nameEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::NAME_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    std::string addr = config.GetHostname();

                    val = addr.c_str();
                    addressLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Address:", ChildId::ADDRESS_LABEL);
                    addressEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::ADDRESS_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetDatabase().c_str();
                    schemaLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Schema name:", ChildId::SCHEMA_LABEL);
                    schemaEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::SCHEMA_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    protocolVersionLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Protocol version:", ChildId::PROTOCOL_VERSION_LABEL);
                    protocolVersionComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        "Protocol version", ChildId::PROTOCOL_VERSION_COMBO_BOX);

                    int id = 0;

                    const ProtocolVersion::VersionSet& supported = ProtocolVersion::GetSupported();

                    ProtocolVersion version = ProtocolVersion::GetCurrent();

                    if (!version.IsSupported())
                        version = ProtocolVersion::GetCurrent();

                    for (ProtocolVersion::VersionSet::const_iterator it = supported.begin(); it != supported.end(); ++it)
                    {
                        protocolVersionComboBox->AddString(it->ToString());

                        if (*it == version)
                            protocolVersionComboBox->SetSelection(id);

                        ++id;
                    }

                    rowPos += INTERVAL + ROW_HEIGHT;

                    connectionSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "Connection settings", ChildId::CONNECTION_SETTINGS_GROUP_BOX);

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateAuthSettingsGroup(int posX, int posY, int sizeX)
                {
                    enum { LABEL_WIDTH = 120 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    const char* val = config.GetUser().c_str();

                    userLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT, "User :", ChildId::USER_LABEL);
                    userEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val, ChildId::USER_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetPassword().c_str();
                    passwordLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Password:", ChildId::PASSWORD_LABEL);
                    passwordEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                        val, ChildId::USER_EDIT, ES_PASSWORD);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    authSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "Authentication settings", ChildId::AUTH_SETTINGS_GROUP_BOX);

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateSslSettingsGroup(int posX, int posY, int sizeX)
                {   // TODO: rename function name from Ssl to TLS after UI works
                    //using ssl::SslMode;

                    enum { LABEL_WIDTH = 120 };

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int rowPos = posY + 2 * INTERVAL;

                    int checkBoxSize = (sizeX - 3 * INTERVAL) / 2;

                    tlsCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                        "TLS", ChildId::TLS_CHECK_BOX, config.IsTls());

                    tlsAllowInvalidHostnamesCheckBox = CreateCheckBox(
                        labelPosX + checkBoxSize + INTERVAL, rowPos,
                        checkBoxSize, ROW_HEIGHT, "TLS Allow Invalid Hostnames",
                        ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX,
                        config.IsTlsAllowInvalidHostnames());

                    rowPos += INTERVAL + ROW_HEIGHT;

                    const char* val = config.GetTlsCaFile().c_str();
                    tlsCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                                    "TLS Certificate Authority:", ChildId::TLS_CA_FILE_LABEL);
                    tlsCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::TLS_CA_FILE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    //SslMode::Type sslMode = ssl::SslMode::REQUIRE;
                    //std::string sslModeStr = SslMode::ToString(sslMode);

                    //const char* val = sslModeStr.c_str();

                    //sslModeLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //    "SSL Mode:", ChildId::SSL_MODE_LABEL);
                    //sslModeComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                    //    "", ChildId::SSL_MODE_COMBO_BOX);

                    //sslModeComboBox->AddString("disable");
                    //sslModeComboBox->AddString("require");

                    //sslModeComboBox->SetSelection(sslMode); // set default value to require -AL-

                    //rowPos += INTERVAL + ROW_HEIGHT; // used to add row hight I believe

                    //val = config.GetTlsCaFile().c_str();
                    //sslKeyFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //    "SSL Private Key:", ChildId::SSL_KEY_FILE_LABEL);
                    //sslKeyFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                    //    val, ChildId::SSL_KEY_FILE_EDIT);

                    //SHAutoComplete(sslKeyFileEdit->GetHandle(), SHACF_DEFAULT);

                    //rowPos += INTERVAL + ROW_HEIGHT;

                    //val = config.GetTlsCaFile().c_str();
                    //sslCertFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //    "SSL Certificate:", ChildId::SSL_CERT_FILE_LABEL);
                    //sslCertFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                    //    val, ChildId::SSL_CERT_FILE_EDIT);

                    //SHAutoComplete(sslCertFileEdit->GetHandle(), SHACF_DEFAULT);

                    // rowPos += INTERVAL + ROW_HEIGHT;

                    //val = config.GetTlsCaFile().c_str();
                    //sslCaFileLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //    "SSL Certificate Authority:", ChildId::SSL_CA_FILE_LABEL);
                    //sslCaFileEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                    //    val, ChildId::SSL_CA_FILE_EDIT);

                    //SHAutoComplete(sslCaFileEdit->GetHandle(), SHACF_DEFAULT);

                    // rowPos += INTERVAL + ROW_HEIGHT;

                    sslSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "TLS/SSL settings", ChildId::SSL_SETTINGS_GROUP_BOX);

                    tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

                    //sslKeyFileEdit->SetEnabled(sslMode != SslMode::DISABLE);
                    //sslCertFileEdit->SetEnabled(sslMode != SslMode::DISABLE);
                    //sslCaFileEdit->SetEnabled(sslMode != SslMode::DISABLE);

                    return rowPos - posY;
                }

                int DsnConfigurationWindow::CreateAdditionalSettingsGroup(int posX, int posY, int sizeX)
                {
                    enum { LABEL_WIDTH = 130 }; // -AL- different definition from above. I can also change it to the same

                    int labelPosX = posX + INTERVAL;

                    int editSizeX = sizeX - LABEL_WIDTH - 3 * INTERVAL;
                    int editPosX = labelPosX + LABEL_WIDTH + INTERVAL;

                    int checkBoxSize = (sizeX - 3 * INTERVAL) / 2;

                    //ProtocolVersion version = ProtocolVersion::GetCurrent();

                    //if (!version.IsSupported())
                    //    version = ProtocolVersion::GetCurrent();

                    int rowPos = posY + 2 * INTERVAL;

                    const char* val = config.GetApplicationName().c_str();

                    appNameLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                        ROW_HEIGHT, "Application Name:", ChildId::APP_NAME_LABEL);
                    appNameEdit = CreateEdit(editPosX, rowPos, editSizeX,
                        ROW_HEIGHT, val, ChildId::APP_NAME_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    std::string tmp = common::LexicalCast< std::string >(config.GetLoginTimeoutSeconds());  
                    val = tmp.c_str(); 
                    loginTimeoutSecLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                        ROW_HEIGHT, "Login Timeout (s):", ChildId::LOGIN_TIMEOUT_SEC_LABEL);

                    loginTimeoutSecEdit = CreateEdit(editPosX, rowPos, editSizeX,
                        ROW_HEIGHT, val, ChildId::LOGIN_TIMEOUT_SEC_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    val = config.GetReadPreference().c_str();

                    readPreferenceLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                        "Read preference:", ChildId::READ_PREFERENCE_LABEL);
                    readPreferenceEdit = CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                                   ChildId::READ_PREFERENCE_EDIT);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    tmp = common::LexicalCast<std::string>(config.GetFetchSize());
                    val = tmp.c_str();
                    fetchSizeLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH,
                        ROW_HEIGHT, "Fetch size:", ChildId::FETCH_SIZE_LABEL);

                    fetchSizeEdit = CreateEdit(editPosX, rowPos, editSizeX,
                        ROW_HEIGHT, val, ChildId::FETCH_SIZE_EDIT, ES_NUMBER);

                    rowPos += INTERVAL + ROW_HEIGHT;

                    //std::string tmp = common::LexicalCast< std::string >(1000);
                    //const char* val = tmp.c_str();
                    //pageSizeLabel =
                    //    CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //                "Page size:", ChildId::PAGE_SIZE_LABEL);

                    //pageSizeEdit =
                    //    CreateEdit(editPosX, rowPos, editSizeX, ROW_HEIGHT, val,
                    //               ChildId::PAGE_SIZE_EDIT, ES_NUMBER);

                    //rowPos += INTERVAL + ROW_HEIGHT;

                    //nestedTxModeLabel = CreateLabel(labelPosX, rowPos, LABEL_WIDTH, ROW_HEIGHT,
                    //    "Nested Transaction Mode:", ChildId::NESTED_TX_MODE_LABEL);
                    //nestedTxModeComboBox = CreateComboBox(editPosX, rowPos, editSizeX, ROW_HEIGHT,
                    //    "", ChildId::NESTED_TX_MODE_COMBO_BOX);

                    /*
                    int id = 0;

                    const NestedTxMode::ModeSet& supported = NestedTxMode::GetValidValues();

                    for (NestedTxMode::ModeSet::const_iterator it = supported.begin(); it != supported.end(); ++it)
                    {
                        nestedTxModeComboBox->AddString(NestedTxMode::ToString(*it));

                        if (*it == config.GetNestedTxMode())
                            nestedTxModeComboBox->SetSelection(id); 

                    //    ++id;
                    //}
                    //*/
                    //nestedTxModeComboBox->SetEnabled(version >= ProtocolVersion::VERSION_2_5_0);

                    //rowPos += INTERVAL + ROW_HEIGHT;

                    //distributedJoinsCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                    //    "Distributed Joins", ChildId::DISTRIBUTED_JOINS_CHECK_BOX, false);

                    //enforceJoinOrderCheckBox = CreateCheckBox(labelPosX + checkBoxSize + INTERVAL,
                    //    rowPos, checkBoxSize, ROW_HEIGHT, "Enforce Join Order",
                    //    ChildId::ENFORCE_JOIN_ORDER_CHECK_BOX, false);

                    //rowPos += ROW_HEIGHT;

                    //replicatedOnlyCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                    //    "Replicated Only", ChildId::REPLICATED_ONLY_CHECK_BOX, false);
                    //
                    //collocatedCheckBox = CreateCheckBox(labelPosX + checkBoxSize + INTERVAL, rowPos, checkBoxSize,
                    //    ROW_HEIGHT, "Collocated", ChildId::COLLOCATED_CHECK_BOX, false);

                    //rowPos += ROW_HEIGHT;

                    //lazyCheckBox = CreateCheckBox(labelPosX, rowPos, checkBoxSize, ROW_HEIGHT,
                    //    "Lazy", ChildId::LAZY_CHECK_BOX, false);

                    ////lazyCheckBox->SetEnabled(version >= ProtocolVersion::VERSION_2_1_5);

                    //skipReducerOnUpdateCheckBox = CreateCheckBox(labelPosX + checkBoxSize + INTERVAL, rowPos,
                    //    checkBoxSize, ROW_HEIGHT, "Skip reducer on update", ChildId::SKIP_REDUCER_ON_UPDATE_CHECK_BOX,
                    //    false);

                    //skipReducerOnUpdateCheckBox->SetEnabled(version >= ProtocolVersion::VERSION_2_3_0);

                    //rowPos += ROW_HEIGHT + INTERVAL;

                    additionalSettingsGroupBox = CreateGroupBox(posX, posY, sizeX, rowPos - posY,
                        "Additional settings", ChildId::ADDITIONAL_SETTINGS_GROUP_BOX);

                    return rowPos - posY;
                }
                
                bool DsnConfigurationWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
                {
                    switch (msg)
                    {
                        case WM_COMMAND:
                        {
                            switch (LOWORD(wParam))
                            {
                                case ChildId::OK_BUTTON:
                                {
                                    try
                                    {
                                        RetrieveParameters(config);

                                        accepted = true;

                                        PostMessage(GetHandle(), WM_CLOSE, 0, 0);
                                    }
                                    catch (IgniteError& err)
                                    {
                                        MessageBox(NULL, err.GetText(), "Error!", MB_ICONEXCLAMATION | MB_OK);
                                    }

                                    break;
                                }

                                case IDCANCEL:
                                case ChildId::CANCEL_BUTTON:
                                {
                                    PostMessage(GetHandle(), WM_CLOSE, 0, 0);

                                    break;
                                }

                                //case ChildId::DISTRIBUTED_JOINS_CHECK_BOX:
                                //{
                                //    distributedJoinsCheckBox->SetChecked(!distributedJoinsCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::ENFORCE_JOIN_ORDER_CHECK_BOX:
                                //{
                                //    enforceJoinOrderCheckBox->SetChecked(!enforceJoinOrderCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::REPLICATED_ONLY_CHECK_BOX:
                                //{
                                //    replicatedOnlyCheckBox->SetChecked(!replicatedOnlyCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::COLLOCATED_CHECK_BOX:
                                //{
                                //    collocatedCheckBox->SetChecked(!collocatedCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::LAZY_CHECK_BOX:
                                //{
                                //    lazyCheckBox->SetChecked(!lazyCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::SKIP_REDUCER_ON_UPDATE_CHECK_BOX:
                                //{
                                //    skipReducerOnUpdateCheckBox->SetChecked(!skipReducerOnUpdateCheckBox->IsChecked());

                                //    break;
                                //}

                                //case ChildId::PROTOCOL_VERSION_COMBO_BOX:
                                //{
                                //    std::string versionStr;
                                //    protocolVersionComboBox->GetText(versionStr);

                                //    ProtocolVersion version = ProtocolVersion::FromString(versionStr);
                                //    lazyCheckBox->SetEnabled(version >= ProtocolVersion::VERSION_2_1_5);
                                //    skipReducerOnUpdateCheckBox->SetEnabled(version >= ProtocolVersion::VERSION_2_3_0);
                                //    nestedTxModeComboBox->SetEnabled(version >= ProtocolVersion::VERSION_2_5_0);

                                //    break;
                                //}

                                case ChildId::TLS_CHECK_BOX: 
                                {
                                    tlsCheckBox->SetChecked(!tlsCheckBox->IsChecked());
                                    tlsCaFileEdit->SetEnabled(tlsCheckBox->IsChecked());

                                    break;
                                }


                                case ChildId::TLS_ALLOW_INVALID_HOSTNAMES_CHECK_BOX:
                                {
                                    tlsAllowInvalidHostnamesCheckBox->SetChecked(!tlsAllowInvalidHostnamesCheckBox->IsChecked());

                                    break;
                                }

                                //case ChildId::SSL_MODE_COMBO_BOX: //-AL- may need to remove this later
                                //{
                                //    using ssl::SslMode;

                                //    std::string sslModeStr;
                                //    sslModeComboBox->GetText(sslModeStr);

                                //    SslMode::Type sslMode = SslMode::FromString(sslModeStr, SslMode::DISABLE);

                                //    sslKeyFileEdit->SetEnabled(sslMode != SslMode::DISABLE);
                                //    sslCertFileEdit->SetEnabled(sslMode != SslMode::DISABLE);
                                //    sslCaFileEdit->SetEnabled(sslMode != SslMode::DISABLE);

                                //    break;
                                //}

                                default:
                                    return false;
                            }

                            break;
                        }

                        case WM_DESTROY:
                        {
                            PostQuitMessage(accepted ? Result::OK : Result::CANCEL);

                            break;
                        }

                        default:
                            return false;
                    }

                    return true;
                }

                void DsnConfigurationWindow::RetrieveParameters(config::Configuration& cfg) const
                {
                    RetrieveConnectionParameters(cfg);
                    RetrieveAuthParameters(cfg);
                    RetrieveSslParameters(cfg);
                    RetrieveAdditionalParameters(cfg);
                }

                void DsnConfigurationWindow::RetrieveConnectionParameters(config::Configuration& cfg) const
                {
                    std::string dsnStr;
                    std::string addressStr;
                    std::string schemaStr;
                    std::string versionStr;

                    nameEdit->GetText(dsnStr);
                    addressEdit->GetText(addressStr);
                    schemaEdit->GetText(schemaStr);
                    protocolVersionComboBox->GetText(versionStr);

                    common::StripSurroundingWhitespaces(addressStr);
                    common::StripSurroundingWhitespaces(dsnStr);
                    // Stripping of whitespaces off the schema skipped intentionally

                    LOG_MSG("Retrieving arguments:");
                    LOG_MSG("DSN:                " << dsnStr);
                    LOG_MSG("Address:            " << addressStr);
                    LOG_MSG("Schema:             " << schemaStr);
                    LOG_MSG("Protocol version:   " << versionStr);

                    if (dsnStr.empty())
                        throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, "DSN name can not be empty.");

                    diagnostic::DiagnosticRecordStorage diag;

                    std::vector<EndPoint> addresses;

                    config::ParseAddress(addressStr, addresses, &diag);

                    if (diag.GetStatusRecordsNumber() > 0)
                    {
                        throw IgniteError(IgniteError::IGNITE_ERR_GENERIC,
                            diag.GetStatusRecord(1).GetMessageText().c_str());
                    }

                    ProtocolVersion version = ProtocolVersion::FromString(versionStr);

                    if (!version.IsSupported())
                        throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, "Protocol version is not supported.");

                    cfg.SetDsn(dsnStr);
                    //cfg.SetAddresses(addresses);
                    cfg.SetDatabase(schemaStr);
                    //cfg.SetProtocolVersion(version);
                }

                void DsnConfigurationWindow::RetrieveAuthParameters(config::Configuration& cfg) const
                {
                    std::string user;
                    std::string password;

                    userEdit->GetText(user);
                    passwordEdit->GetText(password);

                    cfg.SetUser(user);
                    cfg.SetPassword(password);
                }

                void DsnConfigurationWindow::RetrieveSslParameters(config::Configuration& cfg) const
                {

                    bool tls = tlsCheckBox->IsChecked();
                    bool tlsAllowInvalidHostnames = tlsAllowInvalidHostnamesCheckBox->IsChecked();
                    std::string tlsCaStr;

                    tlsCaFileEdit->GetText(tlsCaStr);

                    LOG_MSG("TLS/SSL Encryption:          " << (tls ? "true" : "false"));
                    LOG_MSG("tls Allow Invalid Hostnames: " << (tlsAllowInvalidHostnames ? "true" : "false"));
                    LOG_MSG("TLS CA:                      " << tlsCaStr);

                    cfg.SetTls(tls);
                    cfg.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames);
                    cfg.SetTlsCaFile(tlsCaStr);

                    //std::string sslModeStr;
                    //std::string sslKeyStr;
                    //std::string sslCertStr;
                    //std::string sslCaStr;

                    ////sslModeComboBox->GetText(sslModeStr);
                    ////sslKeyFileEdit->GetText(sslKeyStr);
                    ////sslCertFileEdit->GetText(sslCertStr);
                    ////sslCaFileEdit->GetText(sslCaStr);

                    //LOG_MSG("Retrieving arguments:");
                    //LOG_MSG("SSL Mode:           " << sslModeStr);
                    //LOG_MSG("SSL Key:            " << sslKeyStr);
                    //LOG_MSG("SSL Certificate:    " << sslCertStr);
                    //LOG_MSG("SSL CA:             " << sslCaStr);

                    //ssl::SslMode::Type sslMode = ssl::SslMode::FromString(sslModeStr, ssl::SslMode::DISABLE);

                    //cfg.SetSslMode(sslMode);
                    //cfg.SetSslKeyFile(sslKeyStr);
                    //cfg.SetSslCertFile(sslCertStr);
                    //cfg.SetTlsCaFile(sslCaStr);
                }

                void DsnConfigurationWindow::RetrieveAdditionalParameters(config::Configuration& cfg) const
                {

                    std::string appNameStr;
                    std::string readPreferenceStr;

                    appNameEdit->GetText(appNameStr);
                    readPreferenceEdit->GetText(readPreferenceStr);

                    std::string loginTimeoutSecStr;

                    loginTimeoutSecEdit->GetText(loginTimeoutSecStr);

                    int32_t loginTimeoutSec =
                        common::LexicalCast< int32_t >(loginTimeoutSecStr);

                    if (loginTimeoutSec <= 0)
                        loginTimeoutSec = config.GetLoginTimeoutSeconds();

                    std::string fetchSizeStr;

                    fetchSizeEdit->GetText(fetchSizeStr);

                    int32_t fetchSize =
                        common::LexicalCast< int32_t >(fetchSizeStr);

                    if (fetchSize <= 0)
                        fetchSize = config.GetFetchSize();

                    //std::string pageSizeStr;

                    //pageSizeEdit->GetText(pageSizeStr);

                    //int32_t pageSize = common::LexicalCast<int32_t>(pageSizeStr);

                    //if (pageSize <= 0)
                    //    pageSize = config.GetPageSize();
                    
                    //std::string nestedTxModeStr;

                    //nestedTxModeComboBox->GetText(nestedTxModeStr);

                    // unnecessary code is commented out -AL- 
                    //NestedTxMode::Type mode = NestedTxMode::FromString(nestedTxModeStr, config.GetNestedTxMode());

                    //bool distributedJoins = distributedJoinsCheckBox->IsChecked();
                    //bool enforceJoinOrder = enforceJoinOrderCheckBox->IsChecked();
                    //bool replicatedOnly = replicatedOnlyCheckBox->IsChecked();
                    //bool collocated = collocatedCheckBox->IsChecked();
                    //bool lazy = lazyCheckBox->IsChecked();
                    //bool skipReducerOnUpdate = skipReducerOnUpdateCheckBox->IsChecked();

                    LOG_MSG("Retrieving arguments:");
                    LOG_MSG("App name:                " << appNameStr);
                    LOG_MSG("Login timeout (seconds): " << loginTimeoutSecStr);
                    LOG_MSG("Read preference:         " << readPreferenceStr);
                    LOG_MSG("Fetch size:              " << fetchSize);
                    //LOG_MSG("Nested TX Mode:         " << NestedTxMode::ToString(mode));
                    //LOG_MSG("Distributed Joins:      " << (distributedJoins ? "true" : "false"));
                    //LOG_MSG("Enforce Join Order:     " << (enforceJoinOrder ? "true" : "false"));
                    //LOG_MSG("Replicated only:        " << (replicatedOnly ? "true" : "false"));
                    //LOG_MSG("Collocated:             " << (collocated ? "true" : "false"));
                    //LOG_MSG("Lazy:                   " << (lazy ? "true" : "false"));
                    //LOG_MSG("Skip reducer on update: " << (skipReducerOnUpdate ? "true" : "false"));

                    cfg.SetApplicationName(appNameStr);
                    cfg.SetLoginTimeoutSeconds(loginTimeoutSec);
                    cfg.SetFetchSize(fetchSize);
                    //cfg.SetNestedTxMode(mode);
                    //cfg.SetDistributedJoins(distributedJoins);
                    //cfg.SetEnforceJoinOrder(enforceJoinOrder);
                    //cfg.SetReplicatedOnly(replicatedOnly);
                    //cfg.SetCollocated(collocated);
                    //cfg.SetLazy(lazy);
                    //cfg.SetSkipReducerOnUpdate(skipReducerOnUpdate);
                }
            }
        }
    }
}
