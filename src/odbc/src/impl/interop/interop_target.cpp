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

#include "documentdb/odbc/impl/interop/interop_target.h"
#include "documentdb/odbc/impl/binary/binary_type_updater_impl.h"

using namespace documentdb::odbc::common::concurrent;
using namespace documentdb::odbc::jni::java;
using namespace documentdb::odbc::impl::interop;
using namespace documentdb::odbc::impl::binary;
using namespace documentdb::odbc::binary;

namespace documentdb {
namespace odbc {
namespace impl {
namespace interop {
InteropTarget::InteropTarget(SharedPointer< IgniteEnvironment > env,
                             jobject javaRef)
    : env(env), javaRef(javaRef), skipJavaRefRelease(false) {
  // No-op.
}

InteropTarget::InteropTarget(SharedPointer< IgniteEnvironment > env,
                             jobject javaRef, bool skipJavaRefRelease)
    : env(env), javaRef(javaRef), skipJavaRefRelease(skipJavaRefRelease) {
  // No-op.
}

InteropTarget::~InteropTarget() {
  if (!skipJavaRefRelease) {
    JniContext::Release(javaRef);
  }
}

int64_t InteropTarget::WriteTo(InteropMemory* mem, InputOperation& inOp,
                               DocumentDbError& err) {
  BinaryTypeManager* metaMgr = env.Get()->GetTypeManager();

  int32_t metaVer = metaMgr->GetVersion();

  InteropOutputStream out(mem);
  BinaryWriterImpl writer(&out, metaMgr);

  inOp.ProcessInput(writer);

  out.Synchronize();

  if (metaMgr->IsUpdatedSince(metaVer)) {
    if (!metaMgr->ProcessPendingUpdates(err))
      return 0;
  }

  return mem->PointerLong();
}

void InteropTarget::ReadFrom(InteropMemory* mem, OutputOperation& outOp) {
  InteropInputStream in(mem);
  BinaryReaderImpl reader(&in);

  outOp.ProcessOutput(reader);
}

void InteropTarget::ReadError(InteropMemory* mem, DocumentDbError& err) {
  InteropInputStream in(mem);
  BinaryReaderImpl reader(&in);

  // Reading and skipping error class name.
  reader.ReadObject< std::string >();

  std::string msg = reader.ReadObject< std::string >();

  err = DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC, msg.c_str());
}

bool InteropTarget::OutOp(int32_t opType, InteropMemory& inMem,
                          DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = env.Get()->AllocateMemory();

  int64_t outPtr = inMem.PointerLong();

  if (outPtr) {
    int64_t res = env.Get()->Context()->TargetInStreamOutLong(javaRef, opType,
                                                              outPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
      return res == 1;
  }

  return false;
}

bool InteropTarget::OutOp(int32_t opType, InputOperation& inOp,
                          DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = env.Get()->AllocateMemory();

  int64_t outPtr = WriteTo(mem.Get(), inOp, err);

  if (outPtr) {
    int64_t res = env.Get()->Context()->TargetInStreamOutLong(javaRef, opType,
                                                              outPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
      return res == 1;
  }

  return false;
}

bool InteropTarget::OutOp(int32_t opType, DocumentDbError& err) {
  JniErrorInfo jniErr;

  int64_t res =
      env.Get()->Context()->TargetInLongOutLong(javaRef, opType, 0, &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
    return res == 1;

  return false;
}

bool InteropTarget::InOp(int32_t opType, OutputOperation& outOp,
                         DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > mem = env.Get()->AllocateMemory();

  env.Get()->Context()->TargetOutStream(javaRef, opType,
                                        mem.Get()->PointerLong(), &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
    ReadFrom(mem.Get(), outOp);

    return true;
  }

  return false;
}

jobject InteropTarget::InOpObject(int32_t opType, DocumentDbError& err) {
  JniErrorInfo jniErr;

  jobject res = env.Get()->Context()->TargetOutObject(javaRef, opType, &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  return res;
}

void InteropTarget::OutInOp(int32_t opType, InputOperation& inOp,
                            OutputOperation& outOp, DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > outMem = env.Get()->AllocateMemory();
  SharedPointer< InteropMemory > inMem = env.Get()->AllocateMemory();

  int64_t outPtr = WriteTo(outMem.Get(), inOp, err);

  if (outPtr) {
    env.Get()->Context()->TargetInStreamOutStream(
        javaRef, opType, outPtr, inMem.Get()->PointerLong(), &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
      ReadFrom(inMem.Get(), outOp);
  }
}

void InteropTarget::OutInOpX(int32_t opType, InputOperation& inOp,
                             OutputOperation& outOp, DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > outInMem = env.Get()->AllocateMemory();

  int64_t outInPtr = WriteTo(outInMem.Get(), inOp, err);

  if (outInPtr) {
    int64_t res = env.Get()->Context()->TargetInStreamOutLong(
        javaRef, opType, outInPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS
        && res == OperationResult::AI_SUCCESS)
      ReadFrom(outInMem.Get(), outOp);
    else if (res == OperationResult::AI_NULL)
      outOp.SetNull();
    else if (res == OperationResult::AI_ERROR)
      ReadError(outInMem.Get(), err);
    else
      assert(false);
  }
}

InteropTarget::OperationResult::Type InteropTarget::InStreamOutLong(
    int32_t opType, InteropMemory& outInMem, DocumentDbError& err) {
  JniErrorInfo jniErr;

  int64_t outInPtr = outInMem.PointerLong();

  if (outInPtr) {
    int64_t res = env.Get()->Context()->TargetInStreamOutLong(
        javaRef, opType, outInPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    return static_cast< OperationResult::Type >(res);
  }

  return OperationResult::AI_ERROR;
}

int64_t InteropTarget::InStreamOutLong(int32_t opType, InputOperation& inOp,
                                       DocumentDbError& err) {
  JniErrorInfo jniErr;

  SharedPointer< InteropMemory > outMem = env.Get()->AllocateMemory();
  int64_t outPtr = WriteTo(outMem.Get(), inOp, err);

  if (outPtr) {
    int64_t res = env.Get()->Context()->TargetInStreamOutLong(javaRef, opType,
                                                              outPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    if (jniErr.code == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
      return res;
  }

  return OperationResult::AI_ERROR;
}

jobject InteropTarget::InStreamOutObject(int32_t opType,
                                         InteropMemory& outInMem,
                                         DocumentDbError& err) {
  JniErrorInfo jniErr;

  int64_t outInPtr = outInMem.PointerLong();

  if (outInPtr) {
    jobject res = env.Get()->Context()->TargetInStreamOutObject(
        javaRef, opType, outInPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);

    return res;
  }

  return 0;
}

void InteropTarget::InStreamOutStream(int32_t opType, InteropMemory& inMem,
                                      InteropMemory& outMem, DocumentDbError& err) {
  JniErrorInfo jniErr;

  int64_t inPtr = inMem.PointerLong();
  int64_t outPtr = outMem.PointerLong();

  if (inPtr && outPtr) {
    env.Get()->Context()->TargetInStreamOutStream(javaRef, opType, inPtr,
                                                  outPtr, &jniErr);

    DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                          jniErr.errMsg.c_str(), err);
  }
}

int64_t InteropTarget::OutInOpLong(int32_t opType, int64_t val,
                                   DocumentDbError& err) {
  JniErrorInfo jniErr;

  int64_t res =
      env.Get()->Context()->TargetInLongOutLong(javaRef, opType, val, &jniErr);

  DocumentDbError::SetError(jniErr.code, jniErr.errCls.c_str(),
                        jniErr.errMsg.c_str(), err);

  return res;
}
}  // namespace interop
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
