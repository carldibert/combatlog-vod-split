#pragma once
#include <string>
#include <vector>
#include <map>

class CombatEvents
{
public:
	std::string GetDate();
	CombatEvents();
	CombatEvents(std::string initDate, std::string initTime, std::string initCombatEvent);
private:
	std::string date;
	std::string time;
	std::string combatEvent;
};

class FightLengths
{
public:
	std::string bossName;
	std::string fightStart;
	std::string fightEnd;
	std::string localFightTime;
	std::string vodFightTime;
	FightLengths();
};

class InstanceLength
{
public:
	std::string instanceName;
	std::string instanceStart;
	std::string instanceEnd;
	std::string localInstanceTime;
	std::string vodInstanceTime;
	InstanceLength();
};

class CombatLog
{
public:
	std::vector<CombatEvents> combatEvents;
	std::vector<FightLengths> fightLengths;
	std::vector<InstanceLength> instanceLenghts;
	CombatLog();
};