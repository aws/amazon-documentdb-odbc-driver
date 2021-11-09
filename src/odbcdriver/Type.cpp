/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>
#include "ColumnInfo.h"
#include "ScalarType.h"
#include "Type.h"

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

Type::Type() : 
    m_scalarType(ScalarType::NOT_SET),
    m_scalarTypeHasBeenSet(false),
    m_arrayColumnInfoHasBeenSet(false),
    m_rowColumnInfoHasBeenSet(false)
{
}

Type::Type(JsonView jsonValue) : 
    m_scalarType(ScalarType::NOT_SET),
    m_scalarTypeHasBeenSet(false),
    m_arrayColumnInfoHasBeenSet(false),
    m_rowColumnInfoHasBeenSet(false)
{
  *this = jsonValue;
}

const ColumnInfo& Type::GetArrayColumnInfo() const{ return *m_arrayColumnInfo; }
bool Type::ArrayColumnInfoHasBeenSet() const { return m_arrayColumnInfoHasBeenSet; }
void Type::SetArrayColumnInfo(const ColumnInfo& value) { m_arrayColumnInfoHasBeenSet = true; m_arrayColumnInfo = Aws::MakeShared<ColumnInfo>("Type", value); }
void Type::SetArrayColumnInfo(ColumnInfo&& value) { m_arrayColumnInfoHasBeenSet = true; m_arrayColumnInfo = Aws::MakeShared<ColumnInfo>("Type", std::move(value)); }
Type& Type::WithArrayColumnInfo(const ColumnInfo& value) { SetArrayColumnInfo(value); return *this;}
Type& Type::WithArrayColumnInfo(ColumnInfo&& value) { SetArrayColumnInfo(std::move(value)); return *this;}

Type& Type::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("ScalarType"))
  {
    m_scalarType = ScalarTypeMapper::GetScalarTypeForName(jsonValue.GetString("ScalarType"));

    m_scalarTypeHasBeenSet = true;
  }

  if(jsonValue.ValueExists("ArrayColumnInfo"))
  {
    m_arrayColumnInfo = Aws::MakeShared<ColumnInfo>("Type", jsonValue.GetObject("ArrayColumnInfo"));

    m_arrayColumnInfoHasBeenSet = true;
  }

  if(jsonValue.ValueExists("RowColumnInfo"))
  {
    Array<JsonView> rowColumnInfoJsonList = jsonValue.GetArray("RowColumnInfo");
    for(unsigned rowColumnInfoIndex = 0; rowColumnInfoIndex < rowColumnInfoJsonList.GetLength(); ++rowColumnInfoIndex)
    {
      m_rowColumnInfo.push_back(rowColumnInfoJsonList[rowColumnInfoIndex].AsObject());
    }
    m_rowColumnInfoHasBeenSet = true;
  }

  return *this;
}

JsonValue Type::Jsonize() const
{
  JsonValue payload;

  if(m_scalarTypeHasBeenSet)
  {
   payload.WithString("ScalarType", ScalarTypeMapper::GetNameForScalarType(m_scalarType));
  }

  if(m_arrayColumnInfoHasBeenSet)
  {
   payload.WithObject("ArrayColumnInfo", m_arrayColumnInfo->Jsonize());

  }

  if(m_rowColumnInfoHasBeenSet)
  {
   Array<JsonValue> rowColumnInfoJsonList(m_rowColumnInfo.size());
   for(unsigned rowColumnInfoIndex = 0; rowColumnInfoIndex < rowColumnInfoJsonList.GetLength(); ++rowColumnInfoIndex)
   {
     rowColumnInfoJsonList[rowColumnInfoIndex].AsObject(m_rowColumnInfo[rowColumnInfoIndex].Jsonize());
   }
   payload.WithArray("RowColumnInfo", std::move(rowColumnInfoJsonList));

  }

  return payload;
}