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

#ifndef _DOCUMENTDB_ODBC_IMPL_MODULE_MANAGER
#define _DOCUMENTDB_ODBC_IMPL_MODULE_MANAGER

#include <vector>
#include <sstream>

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/dynamic_load_os.h>
#include <documentdb/odbc/ignite_binding_context.h>
#include <documentdb/odbc/utility.h>

/**
 * @def IGNITE_CACHE_ENTRY_PROCESSOR_INVOKER_NAME
 * Function name in which user Invoke callbacks are registred.
 */
#define IGNITE_MODULE_INIT_CALLBACK_NAME "IgniteModuleInit"

/**
 * Max number of additional init callbacks
 */
#define IGNITE_MODULE_ADDITIONAL_INIT_CALLBACKS_MAX_NUM 100

#define DOCUMENTDB_EXPORTED_CALL extern "C" DOCUMENTDB_IMPORT_EXPORT

namespace documentdb {
namespace odbc {
namespace impl {
/**
 * Module manager.
 * Provides methods to manipulate loadable modules.
 */
class ModuleManager {
  typedef common::dynamic::Module Module;
  typedef void(ModuleInitCallback)(IgniteBindingContext&);

 public:
  /**
   * Constructor.
   *
   * @param context Ignite binding context.
   */
  ModuleManager(const IgniteBindingContext& context)
      : loadedModules(), bindingContext(context) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~ModuleManager() {
    for (std::vector< Module >::iterator it = loadedModules.begin();
         it != loadedModules.end(); ++it)
      it->Unload();
  }

  /**
   * Load module.
   *
   * @param path Module path.
   * @param err Error.
   */
  void LoadModule(const std::wstring& path, DocumentDbError& err) {
    common::dynamic::Module module = common::dynamic::LoadModule(path);

    if (!module.IsLoaded()) {
      err = DocumentDbError(
          DocumentDbError::DOCUMENTDB_ERR_GENERIC,
          ("Can not load module [path=" + utility::ToUtf8(path) + ']').c_str());

      return;
    }

    RegisterModule(module);
  }

  /**
   * Register module in ModuleManager.
   *
   * @param module Module to register.
   */
  void RegisterModule(Module& module) {
    loadedModules.push_back(module);

    ModuleInitCallback* callback = GetModuleInitCallback(module);

    if (callback)
      callback(bindingContext);

    for (int i = 0; i < IGNITE_MODULE_ADDITIONAL_INIT_CALLBACKS_MAX_NUM; ++i) {
      ModuleInitCallback* callback0 =
          GetAdditionalModuleInitCallback(module, i);

      if (!callback0)
        break;

      callback0(bindingContext);
    }
  }

 private:
  DOCUMENTDB_NO_COPY_ASSIGNMENT(ModuleManager);

  /**
   * Get callback that inits Ignite module.
   *
   * @param module Module for which callback should be retrieved.
   * @return Callback if found and null-pointer otherwise.
   */
  static ModuleInitCallback* GetModuleInitCallback(Module& module) {
    return reinterpret_cast< ModuleInitCallback* >(
        module.FindSymbol(IGNITE_MODULE_INIT_CALLBACK_NAME));
  }

  static ModuleInitCallback* GetAdditionalModuleInitCallback(Module& module,
                                                             int num) {
    std::stringstream tmp;

    tmp << IGNITE_MODULE_INIT_CALLBACK_NAME << num;

    return reinterpret_cast< ModuleInitCallback* >(
        module.FindSymbol(tmp.str()));
  }

  /** Collection of loaded modules. */
  std::vector< Module > loadedModules;

  /** Ignite environment. */
  IgniteBindingContext bindingContext;
};
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_MODULE_MANAGER
