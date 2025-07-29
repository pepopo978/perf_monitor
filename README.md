# Overview
Tool to monitor performance stats in 30 second intervals for core game functions as well as addons.  Outputs to perf_monitor.log.

From the timings I've observed it seems like the client game logic each frame is kicked off by CSimpleTop(UIParent I think) OnRender and OnUpdate functions.  It's really hard to follow the exact call flow with how abstracted everything is so this could be wrong, but my `[Total] Render` is based on UIParent render.  If you're wondering why total render time is a lot less than the 30 second measuring window I'm not entirely sure, some of that time is spent idling I assuming waiting for networking updates or gpu renders to finish.

If any of these assumptions I've made are wrong or you find better ways to measure things please let me know or make a pull request :)

# Installation
Grab the latest perf_monitor.dll from https://github.com/pepopo978/perf_monitor/releases and place in the same directory as WoW.exe and then add to dlls.txt.

If you would prefer to compile yourself you will need to get:

boost 1.80 32 bit from https://www.boost.org/users/history/version_1_80_0.html
hadesmem from https://github.com/namreeb/hadesmem
CMakeLists.txt is currently looking for boost at set(BOOST_INCLUDEDIR "C:/software/boost_1_80_0") and hadesmem at set(HADESMEM_ROOT "C:/software/hadesmem-v142-Debug-Win32"). Edit as needed.


# Example
Here's some example output fighting sapphiron with 40 bots:

```
07-13 17:15:59: --------------------------------------------------------------------------------------------------------------------------------------
07-13 17:15:59: --- STATS from 22:25:00 to 22:25:30 ---
07-13 17:15:59: [Total] Render: 23116.71 ms.  Avg fps:  61.30
07-13 17:15:59: OnWorldRender:            74.46% (17212.34 ms)
07-13 17:15:59: All Events:                4.78% ( 1104.35 ms)
07-13 17:15:59: UIParent OnUpdate:         4.01% ( 4739.20 ms)
07-13 17:15:59: All OnUpdates:             3.84% (  887.20 ms)
07-13 17:15:59: CWorldSceneRender:         3.60% (  831.06 ms)
07-13 17:15:59: OnWorldUpdate:             1.82% (  421.87 ms)
07-13 17:15:59: CWorldUpdate:              0.84% (  193.53 ms)
07-13 17:15:59: UnitUpdate:                0.12% (   26.86 ms)
07-13 17:15:59: --- DETAILED STATS ---
07-13 17:15:59: [UIParent OnRender                            ] Calls:     1839, Total: 23116.705 ms, Avg: 12.570 ms, Slowest:  19.841 ms, Fastest:  8.508 ms
07-13 17:15:59: [UIParent OnUpdate                            ] Calls:     1839, Total: 4739.197 ms, Avg:  2.577 ms, Slowest:  15.674 ms, Fastest:  1.155 ms
07-13 17:15:59: [OnWorldRender                                ] Calls:     1839, Total: 17212.337 ms, Avg:  9.360 ms, Slowest:  16.506 ms, Fastest:  6.400 ms
07-13 17:15:59: [OnWorldUpdate                                ] Calls:     1839, Total:  421.872 ms, Avg:  0.229 ms, Slowest:   2.966 ms, Fastest:  0.114 ms
07-13 17:15:59: [CWorldSceneRender                            ] Calls:     1839, Total:  831.062 ms, Avg:  0.452 ms, Slowest:   1.829 ms, Fastest:  0.365 ms
07-13 17:15:59: [CWorldUpdate                                 ] Calls:     1839, Total:  193.535 ms, Avg:  0.105 ms, Slowest:   0.477 ms, Fastest:  0.044 ms
07-13 17:15:59: [UnitUpdate                                   ] Calls:     1839, Total:   26.857 ms, Avg:  0.015 ms, Slowest:   0.082 ms, Fastest:  0.008 ms
07-13 17:15:59: [All Event Handling                           ] Calls:   430824, Total: 1104.345 ms, Avg:  0.003 ms, Slowest:   2.789 ms, Fastest:  0.000 ms
07-13 17:15:59: [All OnUpdates                                ] Calls:   305588, Total:  887.197 ms, Avg:  0.003 ms, Slowest:   3.289 ms, Fastest:  0.000 ms

07-13 17:15:59: --- ADDON/FRAME ONUPDATE PERFORMANCE ---
07-13 17:15:59: [pfUI OnUpdate                                ] Calls:   213623, Total:  391.166 ms, Avg:  0.002 ms, Slowest:   3.289 ms, Fastest:  0.000 ms
07-13 17:15:59: [BetterCharacterStats OnUpdate                ] Calls:     9231, Total:  110.063 ms, Avg:  0.012 ms, Slowest:   0.739 ms, Fastest:  0.000 ms
07-13 17:15:59: [MSBT OnUpdate                                ] Calls:     8383, Total:   99.172 ms, Avg:  0.012 ms, Slowest:   2.442 ms, Fastest:  0.000 ms
07-13 17:15:59: [UIParent OnUpdate                            ] Calls:     1839, Total:   93.802 ms, Avg:  0.051 ms, Slowest:   0.151 ms, Fastest:  0.033 ms
07-13 17:15:59: [Rabuffs OnUpdate                             ] Calls:     2041, Total:   41.511 ms, Avg:  0.020 ms, Slowest:   3.194 ms, Fastest:  0.001 ms
07-13 17:15:59: [ShaguDPS OnUpdate                            ] Calls:     3678, Total:   39.553 ms, Avg:  0.011 ms, Slowest:   0.487 ms, Fastest:  0.000 ms
07-13 17:15:59: [SP_ST_Updater OnUpdate                       ] Calls:     1839, Total:   26.795 ms, Avg:  0.015 ms, Slowest:   0.095 ms, Fastest:  0.007 ms
07-13 17:15:59: [CombatText OnUpdate                          ] Calls:     1839, Total:   26.166 ms, Avg:  0.014 ms, Slowest:   0.216 ms, Fastest:  0.001 ms
07-13 17:15:59: [ChatFrame OnUpdate                           ] Calls:     3678, Total:   19.592 ms, Avg:  0.005 ms, Slowest:   0.115 ms, Fastest:  0.002 ms
07-13 17:15:59: [RackFrame OnUpdate                           ] Calls:     1839, Total:   11.196 ms, Avg:  0.006 ms, Slowest:   0.085 ms, Fastest:  0.002 ms
07-13 17:15:59: [OutfitterUpdateFrame OnUpdate                ] Calls:     1839, Total:    9.103 ms, Avg:  0.005 ms, Slowest:   0.064 ms, Fastest:  0.002 ms
07-13 17:15:59: [TankPlates OnUpdate                          ] Calls:     1839, Total:    7.890 ms, Avg:  0.004 ms, Slowest:   0.069 ms, Fastest:  0.000 ms
07-13 17:15:59: [TrinketMenu_TimersFrame OnUpdate             ] Calls:     1839, Total:    5.660 ms, Avg:  0.003 ms, Slowest:   0.016 ms, Fastest:  0.001 ms
07-13 17:15:59: [AutoMarker OnUpdate                          ] Calls:     1839, Total:    2.099 ms, Avg:  0.001 ms, Slowest:   0.009 ms, Fastest:  0.000 ms
07-13 17:15:59: [SuperWowCombatLogger OnUpdate                ] Calls:     1839, Total:    1.782 ms, Avg:  0.001 ms, Slowest:   0.033 ms, Fastest:  0.000 ms
07-13 17:15:59: [LootBlare OnUpdate                           ] Calls:     1839, Total:    1.645 ms, Avg:  0.001 ms, Slowest:   0.009 ms, Fastest:  0.000 ms
07-13 17:15:59: --- ADDON/FRAME EVENTS PERFORMANCE ---
07-13 17:15:59: [pfUI All Events                              ] Calls:   278835, Total:  592.869 ms, Avg:  0.002 ms, Slowest:   2.789 ms, Fastest:  0.000 ms
07-13 17:15:59: [ShaguDPS All Events                          ] Calls:     1650, Total:   86.485 ms, Avg:  0.052 ms, Slowest:   0.175 ms, Fastest:  0.008 ms
07-13 17:15:59: [BetterCharacterStats All Events              ] Calls:     7318, Total:   70.046 ms, Avg:  0.010 ms, Slowest:   0.707 ms, Fastest:  0.000 ms
07-13 17:15:59: [MSBT All Events                              ] Calls:     3320, Total:   52.151 ms, Avg:  0.016 ms, Slowest:   2.398 ms, Fastest:  0.000 ms
07-13 17:15:59: [RaidFrame All Events                         ] Calls:     1478, Total:    6.846 ms, Avg:  0.005 ms, Slowest:   0.056 ms, Fastest:  0.000 ms
07-13 17:15:59: [PetFrame All Events                          ] Calls:     4028, Total:    5.443 ms, Avg:  0.001 ms, Slowest:   0.021 ms, Fastest:  0.000 ms
07-13 17:15:59: [PartyMemberFrameHealthBar All Events         ] Calls:     6969, Total:    3.232 ms, Avg:  0.000 ms, Slowest:   0.099 ms, Fastest:  0.000 ms
07-13 17:15:59: [AutoMarker All Events                        ] Calls:     1204, Total:    3.193 ms, Avg:  0.003 ms, Slowest:   0.018 ms, Fastest:  0.001 ms
07-13 17:15:59: [TargetFrameHealthBar All Events              ] Calls:     1696, Total:    2.679 ms, Avg:  0.002 ms, Slowest:   0.032 ms, Fastest:  0.000 ms
07-13 17:15:59: [PartyMemberFrameManaBar All Events           ] Calls:     8725, Total:    2.615 ms, Avg:  0.000 ms, Slowest:   0.022 ms, Fastest:  0.000 ms
07-13 17:15:59: [SP_ST_Updater All Events                     ] Calls:     1087, Total:    1.973 ms, Avg:  0.002 ms, Slowest:   0.141 ms, Fastest:  0.001 ms
07-13 17:15:59: [RPLL All Events                              ] Calls:     1085, Total:    1.764 ms, Avg:  0.002 ms, Slowest:   0.015 ms, Fastest:  0.001 ms
07-13 17:15:59: [TankPlates All Events                        ] Calls:     1085, Total:    1.707 ms, Avg:  0.002 ms, Slowest:   0.012 ms, Fastest:  0.001 ms
07-13 17:15:59: [PlayerFrameManaBar All Events                ] Calls:     2241, Total:    1.652 ms, Avg:  0.001 ms, Slowest:   0.039 ms, Fastest:  0.000 ms
07-13 17:15:59: [CombatText All Events                        ] Calls:     2711, Total:    1.466 ms, Avg:  0.001 ms, Slowest:   0.101 ms, Fastest:  0.000 ms
07-13 17:15:59: [PlayerFrameHealthBar All Events              ] Calls:     1765, Total:    1.327 ms, Avg:  0.001 ms, Slowest:   0.024 ms, Fastest:  0.000 ms
07-13 17:15:59: [PartyMemberFramePetFrameHealthBar All Events ] Calls:     6969, Total:    1.165 ms, Avg:  0.000 ms, Slowest:   0.027 ms, Fastest:  0.000 ms
07-13 17:15:59: [PetActionBarFrame All Events                 ] Calls:      935, Total:    0.461 ms, Avg:  0.000 ms, Slowest:   0.006 ms, Fastest:  0.000 ms
07-13 17:15:59: [TargetofTargetFrame All Events               ] Calls:     1028, Total:    0.328 ms, Avg:  0.000 ms, Slowest:   0.004 ms, Fastest:  0.000 ms
07-13 17:15:59: [TargetFrameManaBar All Events                ] Calls:     2096, Total:    0.241 ms, Avg:  0.000 ms, Slowest:   0.002 ms, Fastest:  0.000 ms
07-13 17:15:59: [PetFrameManaBar All Events                   ] Calls:     2096, Total:    0.232 ms, Avg:  0.000 ms, Slowest:   0.002 ms, Fastest:  0.000 ms
07-13 17:15:59: [TargetofTargetManaBar All Events             ] Calls:     2096, Total:    0.197 ms, Avg:  0.000 ms, Slowest:   0.002 ms, Fastest:  0.000 ms
07-13 17:15:59: [PartyMemberFramePetFrame All Events          ] Calls:      452, Total:    0.194 ms, Avg:  0.000 ms, Slowest:   0.002 ms, Fastest:  0.000 ms
07-13 17:15:59: [MultiBarBottomRightButton All Events         ] Calls:       12, Total:    0.166 ms, Avg:  0.014 ms, Slowest:   0.144 ms, Fastest:  0.001 ms
07-13 17:15:59: [TargetofTargetHealthBar All Events           ] Calls:     1495, Total:    0.160 ms, Avg:  0.000 ms, Slowest:   0.005 ms, Fastest:  0.000 ms
07-13 17:15:59: [PetFrameHealthBar All Events                 ] Calls:     1495, Total:    0.158 ms, Avg:  0.000 ms, Slowest:   0.002 ms, Fastest:  0.000 ms
07-13 17:15:59: [RinseFrame All Events                        ] Calls:       28, Total:    0.138 ms, Avg:  0.005 ms, Slowest:   0.008 ms, Fastest:  0.003 ms
07-13 17:15:59: [PetPaperDollFrame All Events                 ] Calls:      193, Total:    0.127 ms, Avg:  0.001 ms, Slowest:   0.003 ms, Fastest:  0.000 ms
07-13 17:15:59: [PaperDollFrame All Events                    ] Calls:      113, Total:    0.108 ms, Avg:  0.001 ms, Slowest:   0.003 ms, Fastest:  0.000 ms
07-13 17:15:59: [CharacterFrame All Events                    ] Calls:      113, Total:    0.105 ms, Avg:  0.001 ms, Slowest:   0.003 ms, Fastest:  0.000 ms
07-13 17:15:59: --- ADDON/FRAME SLOWEST EVENTS REPORT (min .1ms duration) ---
07-13 17:15:59: [AutoMarker] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  UNIT_CASTEVENT                                     Total Duration:    2.855 ms      Count:   1085
07-13 17:15:59:    2.  UNIT_MODEL_CHANGED                                 Total Duration:    0.297 ms      Count:    113
07-13 17:15:59:    3.  CHAT_MSG_ADDON                                     Total Duration:    0.041 ms      Count:      6
07-13 17:15:59: [BetterCharacterStats] Top 10 slowest events (out of 23 total):
07-13 17:15:59:    1.  UNIT_AURA                                          Total Duration:   28.885 ms      Count:    790
07-13 17:15:59:    2.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE      Total Duration:    8.920 ms      Count:    268
07-13 17:15:59:    3.  UNIT_CASTEVENT                                     Total Duration:    8.065 ms      Count:   1085
07-13 17:15:59:    4.  UNIT_COMBAT                                        Total Duration:    5.097 ms      Count:   2992
07-13 17:15:59:    5.  UNIT_HEALTH                                        Total Duration:    4.629 ms      Count:   1478
07-13 17:15:59:    6.  CHAT_MSG_SPELL_AURA_GONE_OTHER                     Total Duration:    3.888 ms      Count:    202
07-13 17:15:59:    7.  UNIT_AURA                                          Total Duration:    3.875 ms      Count:    125
07-13 17:15:59:    8.  CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE         Total Duration:    3.613 ms      Count:    133
07-13 17:15:59:    9.  CHAT_MSG_SPELL_PERIODIC_PARTY_DAMAGE               Total Duration:    1.183 ms      Count:     43
07-13 17:15:59:   10.  CHAT_MSG_SPELL_AURA_GONE_PARTY                     Total Duration:    0.398 ms      Count:     23
07-13 17:15:59: [CharacterFrame] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_PORTRAIT_UPDATE                               Total Duration:    0.105 ms      Count:    113
07-13 17:15:59: [CombatText] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  COMBAT_TEXT_UPDATE                                 Total Duration:    0.707 ms      Count:     11
07-13 17:15:59:    2.  UNIT_MANA                                          Total Duration:    0.545 ms      Count:   1222
07-13 17:15:59:    3.  UNIT_HEALTH                                        Total Duration:    0.214 ms      Count:   1478
07-13 17:15:59: [MSBT] Top 10 slowest events (out of 15 total):
07-13 17:15:59:    1.  CHAT_MSG_SPELL_PARTY_DAMAGE                        Total Duration:   23.466 ms      Count:     61
07-13 17:15:59:    2.  CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE            Total Duration:   14.280 ms      Count:    148
07-13 17:15:59:    3.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS       Total Duration:    5.985 ms      Count:    196
07-13 17:15:59:    4.  CHAT_MSG_SPELL_PERIODIC_SELF_DAMAGE                Total Duration:    2.808 ms      Count:      7
07-13 17:15:59:    5.  CHAT_MSG_COMBAT_PARTY_HITS                         Total Duration:    1.271 ms      Count:     17
07-13 17:15:59:    6.  CHAT_MSG_SPELL_HOSTILEPLAYER_BUFF                  Total Duration:    0.887 ms      Count:     10
07-13 17:15:59:    7.  CHAT_MSG_SPELL_DAMAGESHIELDS_ON_OTHERS             Total Duration:    0.759 ms      Count:      6
07-13 17:15:59:    8.  CHAT_MSG_SPELL_PERIODIC_PARTY_BUFFS                Total Duration:    0.726 ms      Count:     24
07-13 17:15:59:    9.  CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE         Total Duration:    0.586 ms      Count:    133
07-13 17:15:59:   10.  CHAT_MSG_SPELL_SELF_DAMAGE                         Total Duration:    0.536 ms      Count:      8
07-13 17:15:59: [MultiBarBottomRightButton] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  ACTIONBAR_SLOT_CHANGED                             Total Duration:    0.166 ms      Count:     12
07-13 17:15:59: [PaperDollFrame] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_MODEL_CHANGED                                 Total Duration:    0.108 ms      Count:    113
07-13 17:15:59: [PartyMemberFrameHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    3.177 ms      Count:   6912
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.055 ms      Count:     57
07-13 17:15:59: [PartyMemberFrameManaBar] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_RAGE                                          Total Duration:    1.669 ms      Count:   3057
07-13 17:15:59:    2.  UNIT_MANA                                          Total Duration:    0.832 ms      Count:   4900
07-13 17:15:59:    3.  UNIT_ENERGY                                        Total Duration:    0.110 ms      Count:    728
07-13 17:15:59:    4.  UNIT_FOCUS                                         Total Duration:    0.004 ms      Count:      8
07-13 17:15:59:    5.  UNIT_HAPPINESS                                     Total Duration:    0.000 ms      Count:     32
07-13 17:15:59: [PartyMemberFramePetFrame] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_PORTRAIT_UPDATE                               Total Duration:    0.194 ms      Count:    452
07-13 17:15:59: [PartyMemberFramePetFrameHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    1.163 ms      Count:   6912
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.002 ms      Count:     57
07-13 17:15:59: [PetActionBarFrame] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  UNIT_AURA                                          Total Duration:    0.393 ms      Count:    790
07-13 17:15:59:    2.  UNIT_AURA                                          Total Duration:    0.049 ms      Count:    125
07-13 17:15:59:    3.  UNIT_FLAGS                                         Total Duration:    0.019 ms      Count:     20
07-13 17:15:59: [PetFrame] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_COMBAT                                        Total Duration:    5.260 ms      Count:   2992
07-13 17:15:59:    2.  UNIT_AURA                                          Total Duration:    0.102 ms      Count:    790
07-13 17:15:59:    3.  UNIT_PORTRAIT_UPDATE                               Total Duration:    0.053 ms      Count:    113
07-13 17:15:59:    4.  UNIT_HAPPINESS                                     Total Duration:    0.016 ms      Count:      8
07-13 17:15:59:    5.  UNIT_AURA                                          Total Duration:    0.012 ms      Count:    125
07-13 17:15:59: [PetFrameHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    0.157 ms      Count:   1483
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.001 ms      Count:     12
07-13 17:15:59: [PetFrameManaBar] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_MANA                                          Total Duration:    0.150 ms      Count:   1222
07-13 17:15:59:    2.  UNIT_RAGE                                          Total Duration:    0.055 ms      Count:    682
07-13 17:15:59:    3.  UNIT_ENERGY                                        Total Duration:    0.026 ms      Count:    182
07-13 17:15:59:    4.  UNIT_FOCUS                                         Total Duration:    0.001 ms      Count:      2
07-13 17:15:59:    5.  UNIT_HAPPINESS                                     Total Duration:    0.000 ms      Count:      8
07-13 17:15:59: [PetPaperDollFrame] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  UNIT_MODEL_CHANGED                                 Total Duration:    0.086 ms      Count:    113
07-13 17:15:59:    2.  UNIT_ATTACK_SPEED                                  Total Duration:    0.041 ms      Count:     40
07-13 17:15:59:    3.  UNIT_ATTACK_SPEED                                  Total Duration:    0.000 ms      Count:     40
07-13 17:15:59: [PlayerFrameHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    1.324 ms      Count:   1750
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.003 ms      Count:     15
07-13 17:15:59: [PlayerFrameManaBar] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_MANA                                          Total Duration:    1.264 ms      Count:   1255
07-13 17:15:59:    2.  UNIT_RAGE                                          Total Duration:    0.291 ms      Count:    794
07-13 17:15:59:    3.  UNIT_ENERGY                                        Total Duration:    0.091 ms      Count:    182
07-13 17:15:59:    4.  UNIT_HAPPINESS                                     Total Duration:    0.004 ms      Count:      8
07-13 17:15:59:    5.  UNIT_FOCUS                                         Total Duration:    0.002 ms      Count:      2
07-13 17:15:59: [RPLL] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_CASTEVENT                                     Total Duration:    1.764 ms      Count:   1085
07-13 17:15:59: [RaidFrame] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    6.846 ms      Count:   1478
07-13 17:15:59: [RinseFrame] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  SPELL_QUEUE_EVENT                                  Total Duration:    0.138 ms      Count:     28
07-13 17:15:59: [SP_ST_Updater] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  UNIT_CASTEVENT                                     Total Duration:    1.946 ms      Count:   1085
07-13 17:15:59:    2.  CHAT_MSG_SPELL_CREATURE_VS_SELF_DAMAGE             Total Duration:    0.026 ms      Count:      1
07-13 17:15:59:    3.  ACTIONBAR_SLOT_CHANGED                             Total Duration:    0.001 ms      Count:      1
07-13 17:15:59: [ShaguDPS] Top 10 slowest events (out of 18 total):
07-13 17:15:59:    1.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE      Total Duration:   17.796 ms      Count:    268
07-13 17:15:59:    2.  CHAT_MSG_SPELL_FRIENDLYPLAYER_BUFF                 Total Duration:   16.169 ms      Count:    250
07-13 17:15:59:    3.  CHAT_MSG_SPELL_FRIENDLYPLAYER_DAMAGE               Total Duration:   14.410 ms      Count:    263
07-13 17:15:59:    4.  CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE         Total Duration:   10.792 ms      Count:    133
07-13 17:15:59:    5.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS       Total Duration:    7.095 ms      Count:    196
07-13 17:15:59:    6.  CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE            Total Duration:    6.391 ms      Count:    148
07-13 17:15:59:    7.  CHAT_MSG_COMBAT_FRIENDLYPLAYER_HITS                Total Duration:    4.370 ms      Count:    196
07-13 17:15:59:    8.  CHAT_MSG_SPELL_PARTY_DAMAGE                        Total Duration:    3.223 ms      Count:     61
07-13 17:15:59:    9.  CHAT_MSG_SPELL_PERIODIC_PARTY_DAMAGE               Total Duration:    2.492 ms      Count:     43
07-13 17:15:59:   10.  CHAT_MSG_SPELL_PERIODIC_PARTY_BUFFS                Total Duration:    0.874 ms      Count:     24
07-13 17:15:59: [TankPlates] Top 1 slowest events (out of 1 total):
07-13 17:15:59:    1.  UNIT_CASTEVENT                                     Total Duration:    1.707 ms      Count:   1085
07-13 17:15:59: [TargetFrameHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    2.673 ms      Count:   1684
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.006 ms      Count:     12
07-13 17:15:59: [TargetFrameManaBar] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_MANA                                          Total Duration:    0.152 ms      Count:   1222
07-13 17:15:59:    2.  UNIT_RAGE                                          Total Duration:    0.060 ms      Count:    682
07-13 17:15:59:    3.  UNIT_ENERGY                                        Total Duration:    0.028 ms      Count:    182
07-13 17:15:59:    4.  UNIT_FOCUS                                         Total Duration:    0.001 ms      Count:      2
07-13 17:15:59:    5.  UNIT_HAPPINESS                                     Total Duration:    0.000 ms      Count:      8
07-13 17:15:59: [TargetofTargetFrame] Top 3 slowest events (out of 3 total):
07-13 17:15:59:    1.  UNIT_AURA                                          Total Duration:    0.278 ms      Count:    790
07-13 17:15:59:    2.  UNIT_AURA                                          Total Duration:    0.027 ms      Count:    125
07-13 17:15:59:    3.  UNIT_PORTRAIT_UPDATE                               Total Duration:    0.023 ms      Count:    113
07-13 17:15:59: [TargetofTargetHealthBar] Top 2 slowest events (out of 2 total):
07-13 17:15:59:    1.  UNIT_HEALTH                                        Total Duration:    0.159 ms      Count:   1483
07-13 17:15:59:    2.  UNIT_MAXHEALTH                                     Total Duration:    0.001 ms      Count:     12
07-13 17:15:59: [TargetofTargetManaBar] Top 5 slowest events (out of 5 total):
07-13 17:15:59:    1.  UNIT_MANA                                          Total Duration:    0.132 ms      Count:   1222
07-13 17:15:59:    2.  UNIT_RAGE                                          Total Duration:    0.043 ms      Count:    682
07-13 17:15:59:    3.  UNIT_ENERGY                                        Total Duration:    0.021 ms      Count:    182
07-13 17:15:59:    4.  UNIT_FOCUS                                         Total Duration:    0.001 ms      Count:      2
07-13 17:15:59:    5.  UNIT_HAPPINESS                                     Total Duration:    0.000 ms      Count:      8
07-13 17:15:59: [pfUI] Top 10 slowest events (out of 33 total):
07-13 17:15:59:    1.  UNIT_AURA                                          Total Duration:  115.831 ms      Count:  45820
07-13 17:15:59:    2.  UNIT_HEALTH                                        Total Duration:   92.335 ms      Count:  87497
07-13 17:15:59:    3.  UNIT_MANA                                          Total Duration:   74.366 ms      Count:  69654
07-13 17:15:59:    4.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE      Total Duration:   50.728 ms      Count:    268
07-13 17:15:59:    5.  UNIT_RAGE                                          Total Duration:   39.592 ms      Count:  38192
07-13 17:15:59:    6.  CHAT_MSG_SPELL_FRIENDLYPLAYER_DAMAGE               Total Duration:   36.921 ms      Count:    526
07-13 17:15:59:    7.  CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE            Total Duration:   30.755 ms      Count:    296
07-13 17:15:59:    8.  CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS       Total Duration:   26.516 ms      Count:    196
07-13 17:15:59:    9.  CHAT_MSG_SPELL_FRIENDLYPLAYER_BUFF                 Total Duration:   23.818 ms      Count:    250
07-13 17:15:59:   10.  CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE         Total Duration:   21.271 ms      Count:    133
07-13 17:15:59: --- TOTAL EVENT DURATION STATISTICS (SHOULD INCLUDE MISSING ADDONS) ---
07-13 17:15:59: [UNIT_HEALTH                                  ] Calls:     1750, Total:  241.818 ms, Avg:  0.138 ms, Slowest:   0.665 ms, Fastest:  0.107 ms
07-13 17:15:59: [UNIT_AURA                                    ] Calls:      790, Total:  185.553 ms, Avg:  0.235 ms, Slowest:   3.149 ms, Fastest:  0.112 ms
07-13 17:15:59: [UNIT_MANA                                    ] Calls:     1255, Total:  147.270 ms, Avg:  0.117 ms, Slowest:   0.255 ms, Fastest:  0.100 ms
07-13 17:15:59: [UNIT_RAGE                                    ] Calls:      794, Total:   86.577 ms, Avg:  0.109 ms, Slowest:   0.303 ms, Fastest:  0.087 ms
07-13 17:15:59: [CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE] Calls:      268, Total:   78.398 ms, Avg:  0.293 ms, Slowest:   0.571 ms, Fastest:  0.103 ms
07-13 17:15:59: [CHAT_MSG_SPELL_FRIENDLYPLAYER_DAMAGE         ] Calls:      263, Total:   52.416 ms, Avg:  0.199 ms, Slowest:   0.707 ms, Fastest:  0.062 ms
07-13 17:15:59: [CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE      ] Calls:      148, Total:   52.157 ms, Avg:  0.352 ms, Slowest:   0.724 ms, Fastest:  0.152 ms
07-13 17:15:59: [CHAT_MSG_SPELL_FRIENDLYPLAYER_BUFF           ] Calls:      250, Total:   40.658 ms, Avg:  0.163 ms, Slowest:   0.524 ms, Fastest:  0.045 ms
07-13 17:15:59: [CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS ] Calls:      196, Total:   40.409 ms, Avg:  0.206 ms, Slowest:   0.309 ms, Fastest:  0.082 ms
07-13 17:15:59: [CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE   ] Calls:      133, Total:   36.900 ms, Avg:  0.277 ms, Slowest:   1.015 ms, Fastest:  0.134 ms
07-13 17:15:59: --------------------------------------------------------------------------------------------------------------------------------------
```
