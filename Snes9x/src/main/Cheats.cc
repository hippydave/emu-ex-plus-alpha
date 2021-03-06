#include <imagine/gui/TextEntry.hh>
#include <imagine/logger/logger.h>
#include <emuframework/Cheats.hh>
#include <emuframework/EmuApp.hh>
#include "EmuCheatViews.hh"
#include <cheats.h>

void checkAndEnableGlobalCheats()
{
	#ifndef SNES9X_VERSION_1_4
	for(auto &c : Cheat.g)
	{
		if(c.enabled)
		{
			logMsg("cheat processing is enabled");
			Cheat.enabled = true;
			return;
		}
	}
	logMsg("cheat processing is disabled");
	Cheat.enabled = false;
	#endif
}

uint32_t numCheats()
{
	#ifndef SNES9X_VERSION_1_4
	return Cheat.g.size();
	#else
	return Cheat.num_cheats;
	#endif
}

static void setCheatName(uint32_t idx, const char *name)
{
	#ifndef SNES9X_VERSION_1_4
	if(idx >= numCheats())
		return;
	Cheat.g[idx].name = name;
	#else
	string_copy(Cheat.c[idx].name, name);
	#endif
}

static const char *cheatName(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	return Cheat.g[idx].name.c_str();
	#else
	return Cheat.c[idx].name;
	#endif
}

static void deleteCheat(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	S9xDeleteCheatGroup(idx);
	checkAndEnableGlobalCheats();
	#else
	S9xDeleteCheat(idx);
	#endif
}

static bool cheatIsEnabled(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	return Cheat.g[idx].enabled;
	#else
	return Cheat.c[idx].enabled;
	#endif
}

static void enableCheat(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	S9xEnableCheatGroup(idx);
	checkAndEnableGlobalCheats();
	#else
	S9xEnableCheat(idx);
	#endif
}

static void disableCheat(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	S9xDisableCheatGroup(idx);
	checkAndEnableGlobalCheats();
	#else
	S9xDisableCheat(idx);
	#endif
}

static void setCheatAddress(uint32_t idx, uint32_t a)
{
	#ifndef SNES9X_VERSION_1_4
	Cheat.g[idx].c[0].address = a;
	#else
	Cheat.c[idx].address = a;
	#endif
}

static uint32_t cheatAddress(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	return Cheat.g[idx].c[0].address;
	#else
	return Cheat.c[idx].address;
	#endif
}

static void setCheatValue(uint32_t idx, uint8 v)
{
	#ifndef SNES9X_VERSION_1_4
	Cheat.g[idx].c[0].byte = v;
	#else
	Cheat.c[idx].byte = v;
	#endif
}

static uint8 cheatValue(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	return Cheat.g[idx].c[0].byte;
	#else
	return Cheat.c[idx].byte;
	#endif
}

static void setCheatConditionalValue(uint32_t idx, bool conditional, uint8 v)
{
	#ifndef SNES9X_VERSION_1_4
	Cheat.g[idx].c[0].conditional = conditional;
	Cheat.g[idx].c[0].cond_byte = v;
	#else
	Cheat.c[idx].saved = conditional;
	Cheat.c[idx].saved_byte = v;
	#endif
}

static std::pair<bool, uint8> cheatConditionalValue(uint32_t idx)
{
	#ifndef SNES9X_VERSION_1_4
	return {Cheat.g[idx].c[0].conditional, Cheat.g[idx].c[0].cond_byte};
	#else
	return {Cheat.c[idx].saved, Cheat.c[idx].saved_byte};
	#endif
}

static bool addCheat(const char *cheatStr)
{
	#ifndef SNES9X_VERSION_1_4
	if(S9xAddCheatGroup("", cheatStr) == -1)
	{
		return false;
	}
	checkAndEnableGlobalCheats();
	return true;
	#else
	uint8 byte;
	uint32 address;
	uint8 bytes[3];
	bool8 sram;
	uint8 numBytes;
	if(!S9xGameGenieToRaw(cheatStr, address, byte))
	{
		S9xAddCheat(false, true, address, byte);
		return true;
	}
	else if(!S9xProActionReplayToRaw (cheatStr, address, byte))
	{
		S9xAddCheat(false, true, address, byte);
		return true;
	}
	else if(!S9xGoldFingerToRaw(cheatStr, address, sram, numBytes, bytes))
	{
		iterateTimes(numBytes, i)
			S9xAddCheat(false, true, address + i, bytes[i]);
		// TODO: handle cheat names for multiple codes added at once
		return true;
	}
	return false;
	#endif
}

void EmuEditCheatView::renamed(const char *str)
{
	setCheatName(idx, str);
	name.compile(cheatName(idx), renderer(), projP);
}

EmuEditCheatView::EmuEditCheatView(ViewAttachParams attach, uint cheatIdx, RefreshCheatsDelegate onCheatListChanged_):
	BaseEditCheatView
	{
		"Edit Address/Values",
		attach,
		cheatName(cheatIdx),
		[this](const TableView &)
		{
			return 5;
		},
		[this](const TableView &, uint idx) -> MenuItem&
		{
			switch(idx)
			{
				case 0: return name;
				case 1: return addr;
				case 2: return value;
				case 3: return saved;
				default: return remove;
			}
		},
		[this](TextMenuItem &, View &, Input::Event)
		{
			deleteCheat(idx);
			onCheatListChanged();
			dismiss();
			return true;
		},
		onCheatListChanged_
	},
	addr
	{
		"Address",
		addrStr,
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			EmuApp::pushAndShowNewCollectValueInputView<const char*>(attachParams(), e, "Input 6-digit hex", addrStr,
				[this](auto str)
				{
					uint a = strtoul(str, nullptr, 16);
					if(a > 0xFFFFFF)
					{
						logMsg("addr 0x%X too large", a);
						EmuApp::postMessage(true, "Invalid input");
						postDraw();
						return false;
					}
					string_copy(addrStr, a ? str : "0");
					auto wasEnabled = cheatIsEnabled(idx);
					if(wasEnabled)
					{
						disableCheat(idx);
					}
					setCheatAddress(idx, a);
					if(wasEnabled)
					{
						enableCheat(idx);
					}
					addr.compile(renderer(), projP);
					postDraw();
					return true;
				});
		}
	},
	value
	{
		"Value",
		valueStr,
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			EmuApp::pushAndShowNewCollectValueInputView<const char*>(attachParams(), e, "Input 2-digit hex", valueStr,
				[this](const char *str)
				{
					uint a = strtoul(str, nullptr, 16);
					if(a > 0xFF)
					{
						EmuApp::postMessage(true, "value must be <= FF");
						postDraw();
						return false;
					}
					string_copy(valueStr, a ? str : "0");
					auto wasEnabled = cheatIsEnabled(idx);
					if(wasEnabled)
					{
						disableCheat(idx);
					}
					setCheatValue(idx, a);
					if(wasEnabled)
					{
						enableCheat(idx);
					}
					value.compile(renderer(), projP);
					postDraw();
					return true;
				});
		}
	},
	saved
	{
		#ifndef SNES9X_VERSION_1_4
		"Conditional Value",
		#else
		"Saved Value",
		#endif
		savedStr,
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			EmuApp::pushAndShowNewCollectTextInputView(attachParams(), e, "Input 2-digit hex or blank", savedStr,
				[this](CollectTextInputView &view, const char *str)
				{
					if(str)
					{
						uint a = 0x100;
						if(strlen(str))
						{
							uint a = strtoul(str, nullptr, 16);
							if(a > 0xFF)
							{
								EmuApp::postMessage(true, "value must be <= FF");
								window().postDraw();
								return 1;
							}
							string_copy(savedStr, str);
						}
						else
						{
							savedStr[0] = 0;
						}
						auto wasEnabled = cheatIsEnabled(idx);
						if(wasEnabled)
						{
							disableCheat(idx);
						}
						if(a <= 0xFF)
						{
							setCheatConditionalValue(idx, true, a);
						}
						else
						{
							setCheatConditionalValue(idx, false, 0);
						}
						if(wasEnabled)
						{
							enableCheat(idx);
						}
						saved.compile(renderer(), projP);
						window().postDraw();
					}
					view.dismiss();
					return 0;
				});
		}
	},
	idx{cheatIdx}
{
	auto address = cheatAddress(idx);
	auto value = cheatValue(idx);
	auto [saved, savedVal] = cheatConditionalValue(idx);
	logMsg("got cheat with addr 0x%.6x val 0x%.2x saved val 0x%.2x", address, value, savedVal);
	string_printf(addrStr, "%x", address);
	string_printf(valueStr, "%x", value);
	if(!saved)
		savedStr[0] = 0;
	else
		string_printf(savedStr, "%x", savedVal);
}

void EmuEditCheatListView::loadCheatItems()
{
	uint cheats = numCheats();
	cheat.clear();
	cheat.reserve(cheats);
	iterateTimes(cheats, c)
	{
		cheat.emplace_back(cheatName(c),
			[this, c](TextMenuItem &, View &, Input::Event e)
			{
				pushAndShow(makeView<EmuEditCheatView>(c, [this](){ onCheatListChanged(); }), e);
			});
	}
}

EmuEditCheatListView::EmuEditCheatListView(ViewAttachParams attach):
	BaseEditCheatListView
	{
		attach,
		[this](const TableView &)
		{
			return 1 + cheat.size();
		},
		[this](const TableView &, uint idx) -> MenuItem&
		{
			switch(idx)
			{
				case 0: return addCode;
				default: return cheat[idx - 1];
			}
		}
	},
	addCode
	{
		"Add Game Genie/Action Replay/Gold Finger Code",
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			if(numCheats() == EmuCheats::MAX)
			{
				EmuApp::postMessage(true, "Too many cheats, delete some first");
				return;
			}
			EmuApp::pushAndShowNewCollectTextInputView(attachParams(), e,
				"Input xxxx-xxxx (GG), xxxxxxxx (AR), or GF code", "",
				[this](CollectTextInputView &view, const char *str)
				{
					if(str)
					{
						if(!addCheat(str))
						{
							EmuApp::postMessage(true, "Invalid format");
							return 1;
						}
						auto idx = numCheats() - 1;
						setCheatName(idx, "Unnamed Cheat");
						logMsg("added new cheat, %d total", numCheats());
						onCheatListChanged();
						view.dismiss();
						EmuApp::pushAndShowNewCollectTextInputView(attachParams(), {}, "Input description", "",
							[this, idx](CollectTextInputView &view, const char *str)
							{
								if(str)
								{
									setCheatName(idx, str);
									onCheatListChanged();
									view.dismiss();
								}
								else
								{
									view.dismiss();
								}
								return 0;
							});
					}
					else
					{
						view.dismiss();
					}
					return 0;
				});
		}
	}
{
	loadCheatItems();
}

EmuCheatsView::EmuCheatsView(ViewAttachParams attach): BaseCheatsView{attach}
{
	loadCheatItems();
}

void EmuCheatsView::loadCheatItems()
{
	uint cheats = numCheats();
	cheat.clear();
	cheat.reserve(cheats);
	iterateTimes(cheats, c)
	{
		cheat.emplace_back(cheatName(c), cheatIsEnabled(c),
			[this, c](BoolMenuItem &item, View &, Input::Event e)
			{
				bool on = item.flipBoolValue(*this);
				if(on)
					enableCheat(c);
				else
					disableCheat(c);
			});
	}
}
