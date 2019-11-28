// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "vectors.h"

#include <cppargparse/argparse-s.h>

#include <gtest/gtest.h>

using namespace argparse;
using namespace testing;

TEST( OptionFactoryTest, shouldCreateASimpleOption )
{
   int i;
   OptionFactory factory;
   auto option = factory.createOption( i );

   EXPECT_FALSE( option.hasVectorValue() );
}

TEST( OptionFactoryTest, shouldCreateAVectorOption )
{
   std::vector<int> v;
   OptionFactory factory;
   auto option = factory.createOption( v );

   EXPECT_TRUE( option.hasVectorValue() );
}

TEST( OptionFactoryTest, shouldCreateAVoidValueOption )
{
   VoidValue i;
   OptionFactory factory;
   auto option = factory.createOption( i );

   EXPECT_FALSE( option.hasVectorValue() );
}

TEST( OptionFactoryTest, shouldFailToCreateAVectorOfValues )
{
   std::vector<VoidValue> v;
   OptionFactory factory;
   EXPECT_THROW( factory.createOption( v ), UnsupportedTargetType );
}
