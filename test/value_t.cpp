// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <gtest/gtest.h>

using namespace argparse;
using namespace testing;

namespace {
struct TestStructure
{
   int shared = 0;
   TestStructure() = default;
   TestStructure( const TestStructure& ) = default;
   TestStructure( const std::string& v )
   {}
   TestStructure& operator=( const TestStructure& ) = default;
   TestStructure& operator=( const std::string& v )
   {
      return *this;
   }
};
}   // namespace

TEST( ValueTest, shouldAssignUniqueValueTypeId )
{
   int i;
   unsigned u;
   double d;
   bool b;
   char c;
   TestStructure t;
   auto v0 = VoidValue();
   auto vi = ConvertedValue( i );
   auto vu = ConvertedValue( u );
   auto vd = ConvertedValue( d );
   auto vb = ConvertedValue( b );
   auto vc = ConvertedValue( c );
   auto vt = ConvertedValue( t );
   std::vector<Value*> all{ &v0, &vi, &vu, &vd, &vb, &vc, &vt };

   for ( auto pv1 : all )
      for ( auto pv2 : all )
         if ( pv1 != pv2 )
            EXPECT_TRUE( pv1->getValueTypeId() != pv2->getValueTypeId() );
}

TEST( ValueTest, shouldCastVoidValueToVoidValue )
{
   auto v = VoidValue();
   EXPECT_NE( nullptr, VoidValue::value_cast( v ) );
}

TEST( ValueTest, shouldCastConvertedValueToConvertedValue )
{
   int i;
   auto v = ConvertedValue( i );
   EXPECT_NE( nullptr, ConvertedValue<int>::value_cast( v ) );
}

TEST( ValueTest, shouldNotCastVoidValueToConvertedValue )
{
   auto v = VoidValue();
   EXPECT_EQ( nullptr, ConvertedValue<int>::value_cast( v ) );
}

TEST( ValueTest, shouldNotCastConvertedValueToVoidValue )
{
   int i;
   auto v = ConvertedValue( i );
   EXPECT_EQ( nullptr, VoidValue::value_cast( v ) );
}

TEST( ValueTest, shouldNotCastIntConvertedValueToUnsignedConvertedValue )
{
   int i;
   auto v = ConvertedValue( i );
   EXPECT_EQ( nullptr, ConvertedValue<unsigned>::value_cast( v ) );
}
