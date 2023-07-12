#include "Game/ActorUID.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const ActorUID ActorUID::INVALID = ActorUID(0xFFFFFFFF, 0xFFFFFFFF);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ActorUID::ActorUID(unsigned int index, unsigned int salt)
{
	data = (salt << 16) | (index & 0xFFFF);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ActorUID::ActorUID()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ActorUID::IsValid() const
{
	return *this != INVALID;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int ActorUID::GetIndex() const
{
	return data & 0xFFFF;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ActorUID::operator!=(const ActorUID& other) const
{
	return data != other.data;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ActorUID::operator==(const ActorUID& other) const
{
	return data == other.data;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------