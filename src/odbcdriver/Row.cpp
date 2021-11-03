﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>
#include "Row.h"

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

Row::Row() : 
    m_dataHasBeenSet(false)
{
}

Row::Row(JsonView jsonValue) : 
    m_dataHasBeenSet(false)
{
  *this = jsonValue;
}

Row& Row::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("Data"))
  {
    Array<JsonView> dataJsonList = jsonValue.GetArray("Data");
    for(unsigned dataIndex = 0; dataIndex < dataJsonList.GetLength(); ++dataIndex)
    {
      m_data.push_back(dataJsonList[dataIndex].AsObject());
    }
    m_dataHasBeenSet = true;
  }

  return *this;
}

JsonValue Row::Jsonize() const
{
  JsonValue payload;

  if(m_dataHasBeenSet)
  {
   Array<JsonValue> dataJsonList(m_data.size());
   for(unsigned dataIndex = 0; dataIndex < dataJsonList.GetLength(); ++dataIndex)
   {
     dataJsonList[dataIndex].AsObject(m_data[dataIndex].Jsonize());
   }
   payload.WithArray("Data", std::move(dataJsonList));

  }

  return payload;
}
