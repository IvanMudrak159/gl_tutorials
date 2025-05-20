#pragma once


// Represents a line segment in 3D space with thickness at each end
struct LineSegment {
    glm::vec3 start;
    float startThickness;
    glm::vec3 end;
    float endThickness;
};

struct State {
	glm::mat3 transformation = glm::mat3(1.0f);
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	float shorteningFactor = 1.0;
	float level = 0;

	glm::vec3 localX() { return glm::vec3(transformation[0]); }
	glm::vec3 localY() { return glm::vec3(transformation[1]); }
	glm::vec3 localZ() { return glm::vec3(transformation[2]); }
};

inline glm::mat3 rotationMatrix(glm::vec3 axis, float angleRadians) {
    return glm::mat3(glm::rotate(glm::mat4(1.0f), angleRadians, axis));
}


// Generates the L-system geometry (list of lines) from the generated string
inline std::vector<LineSegment> generateLSystemGeometry(
		const std::string& lSystemString,
		float angle,
		float stepLength,
		float shorteningFactor) {
	std::vector<LineSegment> lines;
	std::stack<State> transformStack;
	State currentState;
	glm::vec3 direction = glm::vec3(0.0f, stepLength, 0.0f);

	for (const char& command : lSystemString) {
		switch (command) {
		case 'F': {
				glm::vec3 newDirection = currentState.transformation * direction;
				glm::vec3 endPosition = currentState.position + currentState.shorteningFactor * newDirection;
				// std::cout << "LINE " << glm::length(newEnd) << " : " << glm::to_string(currentState.position) << " - " << glm::to_string(endPosition) << "\n";
				// std::cout << "CURRENT_TRANSFORM "<< glm::to_string(currentState.transformation) << "\n";
				lines.push_back(LineSegment{
					currentState.position, currentState.level,
					endPosition, currentState.level + 1.0f});
				currentState.position = endPosition;
				currentState.shorteningFactor *= shorteningFactor;
				currentState.level = currentState.level + 1.0f;
				break;
			  }
		case 'I':
			currentState.level = currentState.level + 1.0f;
			break;
		case 'S':
			currentState.shorteningFactor *= shorteningFactor;
			break;
		case '+':
			// Turn right around local Z-axis
			currentState.transformation = rotationMatrix(currentState.localZ(), glm::radians(angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(angle), currentState.localZ());
			break;
		case '-':
			// Turn left around local Z-axis
			currentState.transformation = rotationMatrix(currentState.localZ(), glm::radians(-angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-angle), currentState.localZ());
			break;
		case '>':
			// Pitch down around local X-axis
			currentState.transformation = rotationMatrix(currentState.localX(), glm::radians(angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(angle), currentState.localX());
			break;
		case '<':
			// Pitch up around local X-axis
			currentState.transformation = rotationMatrix(currentState.localX(), glm::radians(-angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-angle), currentState.localX());
			break;
		case '/':
			// Roll right around local Y-axis
			currentState.transformation = rotationMatrix(currentState.localY(), glm::radians(angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(angle), currentState.localY());
			break;
		case '\\':
			// Roll left around local Y-axis
			currentState.transformation = rotationMatrix(currentState.localY(), glm::radians(-angle)) * currentState.transformation;
			// currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-angle), currentState.localY());
			break;
		// case '+':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)); // Turn right around Z-axis
		// 	break;
		// case '-':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-angle), glm::vec3(0.0f, 0.0f, 1.0f)); // Turn left around Z-axis
		// 	break;
		// case '>':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch down around X-axis
		// 	break;
		// case '<':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-angle), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch up around X-axis
		// 	break;
		// case '/':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Roll right around Y-axis
		// 	break;
		// case '\\':
		// 	currentState.transformation = glm::rotate(currentState.transformation, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Roll left around Y-axis
		// 	break;
		case '[':
			transformStack.push(currentState);
			break;
		case ']':
			if (!transformStack.empty()) {
				currentState = transformStack.top();
				transformStack.pop();
			} else {
				std::cerr << "Warning: Transform stack underflow!" << std::endl;

			}	default:
			// Ignore unrecognized characters
			break;
		}
	}

	return lines;
}
