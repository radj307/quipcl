#pragma once
#include <optional>

static struct {
	bool quiet{ false };
	std::optional<size_t> preview_width{ 120ull }, preview_lines{ 3ull };
} Config;
