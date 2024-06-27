#include "CombatLog.h"
#include <iostream>


CombatEvents::CombatEvents()
{

};

CombatEvents::CombatEvents(std::string initDate, std::string initTime, std::string initCombatEvent)
{
	this->date = initDate;
	this->time = initTime;
	this->combatEvent = initCombatEvent;
};

std::string CombatEvents::GetDate()
{
	return date;
}

FightLengths::FightLengths()
{

};

InstanceLength::InstanceLength()
{

};

CombatLog::CombatLog()
{

};