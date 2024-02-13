#pragma once
#include <optional>

class CoordinateIteration{
private:
	const std::vector<std::pair<double, double>>& coordinates;
	size_t currentIndex;

public: 
	CoordinateIteration(const std::vector<std::pair<double, double>>& coords): coordinates(coords), currentIndex(0){}

	std::optional<std::pair<double, double>> getNext() {
		if(currentIndex < coordinates.size()) {
			return coordinates[currentIndex++];
		} else {
			return std::nullopt;
		}
	}


};