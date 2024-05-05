#pragma once

struct QueueFamilyIndicies {
	int graphicsFamily = -1;

	bool isValid() {
		return graphicsFamily >= 0;
	}
};