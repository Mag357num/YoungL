#pragma once
#include <string>

class UObject
{
public:
	UObject(std::string TypeName)
	{
		std::string LinkStr = "_";
		ObjectName = TypeName + LinkStr;

		int RandomInt = rand();
		ObjectName += to_string(RandomInt);
	}

	virtual ~UObject()
	{

	}

	std::string GetObjectName()
	{
		return ObjectName;
	}

	virtual void Serialize() = 0;

	virtual void PostLoad() = 0;

	virtual void Destroy() = 0;

private:
	std::string ObjectName;
};

