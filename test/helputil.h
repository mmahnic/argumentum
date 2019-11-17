// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace helputil {

enum EFlag { KEEPEMPTY = true };

std::vector<std::string_view> splitLines( std::string_view text, bool keepEmpty = false );
bool strHasText( std::string_view line, std::string_view text );
bool strHasTexts( std::string_view line, std::vector<std::string_view> texts );

}   // namespace helputil
