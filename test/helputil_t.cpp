// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "helputil.h"

#include <gtest/gtest.h>

using namespace helputil;

TEST( Utility_strHasText, shouldFindTextInString )
{
   auto line = "some short line";
   EXPECT_TRUE( strHasText( line, "some" ) );
   EXPECT_TRUE( strHasText( line, "short" ) );
   EXPECT_TRUE( strHasText( line, "line" ) );
   EXPECT_FALSE( strHasText( line, "long" ) );
}

TEST( Utility_strHasTexts, shouldFindMultipleTextsInString )
{
   auto line = "some short line";
   EXPECT_TRUE( strHasTexts( line, { "some" } ) );
   EXPECT_TRUE( strHasTexts( line, { "some", "short" } ) );
   EXPECT_TRUE( strHasTexts( line, { "some", "line" } ) );
   EXPECT_TRUE( strHasTexts( line, { "line" } ) );
   EXPECT_FALSE( strHasTexts( line, { "long" } ) );
}

TEST( Utility_strHasTexts, shouldFindMultipleTextsInStringInOrder )
{
   auto line = "some short line";
   EXPECT_TRUE( strHasTexts( line, { "some" } ) );
   EXPECT_TRUE( strHasTexts( line, { "some", "short" } ) );
   EXPECT_FALSE( strHasTexts( line, { "short", "some" } ) );
   EXPECT_FALSE( strHasTexts( line, { "line", "line" } ) );
}
