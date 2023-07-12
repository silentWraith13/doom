#pragma once

class ActorUID 
{
public:
	ActorUID();
	ActorUID(unsigned int index, unsigned int salt);
	bool IsValid() const;
	unsigned int GetIndex() const;
	bool operator==(const ActorUID& other) const;
	bool operator!=(const ActorUID& other) const;

	unsigned int data;
	static const ActorUID INVALID;

};