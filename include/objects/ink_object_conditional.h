#pragma once

#include "objects/ink_object.h"

#include <string>

class InkObjectConditional : public InkObject {
private:
	std::string condition;
	std::vector<InkObject*> branch_if;
	std::vector<InkObject*> branch_else;

public:
	InkObjectConditional(const std::string& condition, const std::vector<InkObject*>& objects_if, const std::vector<InkObject*>& objects_else)
		: condition{condition}, branch_if{objects_if}, branch_else{objects_else} {}
 
	virtual ObjectId get_id() const override { return ObjectId::Conditional; }
};
