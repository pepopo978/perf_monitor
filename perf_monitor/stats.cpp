#include "stats.hpp"
#include "logging.hpp"
#include "events.hpp"
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace perf_monitor {

    // Create stats objects for each monitored function
    FunctionStats gRenderWorldStats("RenderWorld");
    FunctionStats gOnWorldRenderStats("OnWorldRender");
    FunctionStats gOnWorldUpdateStats("OnWorldUpdate");
    FunctionStats gSpellVisualsRenderStats("SpellVisualsRender");
    FunctionStats gSpellVisualsTickStats("SpellVisualsTick");
    FunctionStats gUnitUpdateStats("UnitUpdate");
    FunctionStats gFrameOnLayerUpdateStats("All OnUpdates");
    FunctionStats gCWorldRenderStats("CWorldRender");
    FunctionStats gCWorldUpdateStats("CWorldUpdate");
    FunctionStats gCWorldSceneRenderStats("CWorldSceneRender");
    FunctionStats gCWorldUnknownRenderStats("CWorldUnknownRender");
    FunctionStats gTimeBetweenRenderStats("TimeBetweenRender");
    FunctionStats gFrameOnScriptEventStats("All Event Handling");
    FunctionStats gCSimpleFrameOnFrameRender1Stats("CSimpleFrameOnFrameRender1");
    FunctionStats gCSimpleFrameOnFrameRender2Stats("CSimpleFrameOnFrameRender2");
    FunctionStats gCSimpleModelOnFrameRenderStats("CSimpleModelOnFrameRender");
    FunctionStats gCSimpleTopOnLayerUpdateStats("UIParent OnUpdate");
    FunctionStats gCSimpleTopOnLayerRenderStats("UIParent OnRender");
    FunctionStats gObjectUpdateHandlerStats("ObjectUpdateHandler");
    FunctionStats gPlaySpellVisualStats("PlaySpellVisual");
    FunctionStats gUnknownOnRender1Stats("UnknownOnRender1");
    FunctionStats gUnknownOnRender2Stats("UnknownOnRender2");
    FunctionStats gUnknownOnRender3Stats("UnknownOnRender3");
    FunctionStats gCM2SceneAdvanceTimeStats("CM2Scene::AdvanceTime");
    FunctionStats gCM2SceneAnimateStats("CM2Scene::Animate");
    FunctionStats gCM2SceneDrawStats("CM2Scene::Draw");
    FunctionStats gPaintScreenStats("PaintScreen");
    FunctionStats gDrawBatchProjStats("CM2SceneRender::DrawBatchProj");
    FunctionStats gDrawBatchStats("CM2SceneRender::DrawBatch");
    FunctionStats gDrawBatchDoodadStats("CM2SceneRender::DrawBatchDoodad");
    FunctionStats gDrawRibbonStats("CM2SceneRender::DrawRibbon");
    FunctionStats gDrawParticleStats("CM2SceneRender::DrawParticle");
    FunctionStats gDrawCallbackStats("CM2SceneRender::DrawCallback");
    FunctionStats gCM2SceneRenderDrawStats("CM2SceneRender::Draw");
    FunctionStats gCM2ModelAnimateMTStats("CM2Model::AnimateMT");
    FunctionStats gLuaCCollectgarbageStats("Lua Garbage Collection");
    FunctionStats gObjectFreeStats("World Object Garbage Collection");

    void FunctionStats::update(long long duration) {
        double d = static_cast<double>(duration);
        // Update stats
        callCount++;
        totalTime += d;
        avgTime = static_cast<double>(totalTime) / callCount;

        // Update fastest/slowest times
        if (d > slowestTime) slowestTime = d;
        if (d < fastestTime) fastestTime = d;
    }

    void FunctionStats::outputStats() {
        outputStats(45);
    }
    
    void FunctionStats::outputStats(int nameWidth) {
        DEBUG_LOG(
                std::fixed << std::setprecision(3)
                           << "[" << std::left << std::setw(nameWidth) << name << "] "
                           << "Calls: " << std::right << std::setw(8) << callCount
                           << ", Total: " << std::right << std::setw(8) << totalTime / 1000.0 << " ms"
                           << ", Avg: " << std::right << std::setw(6) << avgTime / 1000.0 << " ms"
                           << ", Slowest: " << std::right << std::setw(7) << slowestTime / 1000.0 << " ms"
                           << ", Fastest: " << std::right << std::setw(6)
                           << (fastestTime == 999999999LL ? 0 : fastestTime / 1000.0) << " ms"
        );
    }

    void FunctionStats::clearStats() {
        // Reset all stats after output
        totalTime = 0;
        callCount = 0;
        avgTime = 0.0;
        slowestTime = 0;
        fastestTime = 999999999LL;
    }

    bool FunctionStats::checkAndOutputStats(uint64_t nowMs) {
        if (periodStartTime == 0) {
            periodStartTime = nowMs;
            return false;
        }
        if (nowMs - periodStartTime >= STATS_OUTPUT_INTERVAL_MS) {
            return true;
        }
        return false;
    }

    void OutputStats(uint64_t startTime, uint64_t endTime) {
        auto callCount = gRenderWorldStats.callCount;

        // Store the totals before clearing the stats
        double totalCSimpleTopOnLayerUpdate = gCSimpleTopOnLayerUpdateStats.totalTime;
        double totalCSimpleTopOnLayerRender = gCSimpleTopOnLayerRenderStats.totalTime;
        double totalPaintScreen = gPaintScreenStats.totalTime;

        double totalOnWorldRender = gOnWorldRenderStats.totalTime;
        double totalOnWorldUpdate = gOnWorldUpdateStats.totalTime;
        double totalSpellVisualsRender = gSpellVisualsRenderStats.totalTime;
        double totalSpellVisualsTick = gSpellVisualsTickStats.totalTime;
        double totalUnitUpdate = gUnitUpdateStats.totalTime;
        double totalObjectUpdateHandler = gObjectUpdateHandlerStats.totalTime;
        double totalPlaySpellVisual = gPlaySpellVisualStats.totalTime;
        double totalUnknownOnRender1 = gUnknownOnRender1Stats.totalTime;
        double totalUnknownOnRender2 = gUnknownOnRender2Stats.totalTime;
        double totalUnknownOnRender3 = gUnknownOnRender3Stats.totalTime;
        double totalCM2SceneAdvanceTime = gCM2SceneAdvanceTimeStats.totalTime;
        double totalCM2SceneAnimate = gCM2SceneAnimateStats.totalTime;
        double totalCM2SceneDraw = gCM2SceneDrawStats.totalTime;
        double totalDrawBatchProj = gDrawBatchProjStats.totalTime;
        double totalDrawBatch = gDrawBatchStats.totalTime;
        double totalDrawBatchDoodad = gDrawBatchDoodadStats.totalTime;
        double totalDrawRibbon = gDrawRibbonStats.totalTime;
        double totalDrawParticle = gDrawParticleStats.totalTime;
        double totalDrawCallback = gDrawCallbackStats.totalTime;
        double totalCM2SceneRenderDraw = gCM2SceneRenderDrawStats.totalTime;
        double totalFrameOnLayerUpdate = gFrameOnLayerUpdateStats.totalTime;
        double totalCWorldRender = gCWorldRenderStats.totalTime;
        double totalCWorldUpdate = gCWorldUpdateStats.totalTime;
        double totalCWorldSceneRender = gCWorldSceneRenderStats.totalTime;
        double totalCWorldUnknownRender = gCWorldUnknownRenderStats.totalTime;
        double totalTimeBetweenRender = gTimeBetweenRenderStats.totalTime;

        double totalFrameOnScriptEvent = gFrameOnScriptEventStats.totalTime;
        double totalCSimpleFrameOnFrameRender1 = gCSimpleFrameOnFrameRender1Stats.totalTime;
        double totalCSimpleFrameOnFrameRender2 = gCSimpleFrameOnFrameRender2Stats.totalTime;
        double totalCSimpleModelOnFrameRender = gCSimpleModelOnFrameRenderStats.totalTime;

        // Get the total time for evt_Paint and evt_Idle
        double totalEvtPaint = gEventStats[EVENT_ID_PAINT].totalTime;
        double totalEvtIdle = gEventStats[EVENT_ID_IDLE].totalTime;
        double totalEvtInit = gEventStats[EVENT_ID_INITIALIZE].totalTime;
        double totalEvtPoll = gEventStats[EVENT_ID_POLL].totalTime;

        // Calculate percentages of frame time using cumulative times
        double onWorldRenderPercent = (totalPaintScreen > 0) ?
                                      (totalOnWorldRender / totalPaintScreen) * 100.0 : 0.0;
        double onWorldUpdatePercent = (totalPaintScreen > 0) ?
                                      (totalOnWorldUpdate / totalPaintScreen) * 100.0 : 0.0;
        double spellVisualsRenderPercent = (totalPaintScreen > 0) ?
                                           (totalSpellVisualsRender / totalPaintScreen) * 100.0 : 0.0;
        double spellVisualsTickPercent = (totalPaintScreen > 0) ?
                                         (totalSpellVisualsTick / totalPaintScreen) * 100.0 : 0.0;
        double unitUpdatePercent = (totalPaintScreen > 0) ?
                                   (totalUnitUpdate / totalPaintScreen) * 100.0 : 0.0;
        double objectUpdateHandlerPercent = (totalEvtPoll > 0) ?
                                            (totalObjectUpdateHandler / totalEvtPoll) * 100.0 : 0.0;
        double playSpellVisualPercent = (totalPaintScreen > 0) ?
                                        (totalPlaySpellVisual / totalPaintScreen) * 100.0 : 0.0;
        double unknownOnRender1Percent = (totalPaintScreen > 0) ?
                                         (totalUnknownOnRender1 / totalPaintScreen) * 100.0 : 0.0;
        double unknownOnRender2Percent = (totalPaintScreen > 0) ?
                                         (totalUnknownOnRender2 / totalPaintScreen) * 100.0 : 0.0;
        double unknownOnRender3Percent = (totalPaintScreen > 0) ?
                                         (totalUnknownOnRender3 / totalPaintScreen) * 100.0 : 0.0;
        double cM2SceneAdvanceTimePercent = (totalPaintScreen > 0) ?
                                            (totalCM2SceneAdvanceTime / totalPaintScreen) * 100.0 : 0.0;
        double cM2SceneAnimatePercent = (totalPaintScreen > 0) ?
                                        (totalCM2SceneAnimate / totalPaintScreen) * 100.0 : 0.0;
        double cM2SceneDrawPercent = (totalPaintScreen > 0) ?
                                     (totalCM2SceneDraw / totalPaintScreen) * 100.0 : 0.0;
        double drawBatchProjPercent = (totalPaintScreen > 0) ?
                                      (totalDrawBatchProj / totalPaintScreen) * 100.0 : 0.0;
        double drawBatchPercent = (totalPaintScreen > 0) ?
                                  (totalDrawBatch / totalPaintScreen) * 100.0 : 0.0;
        double drawBatchDoodadPercent = (totalPaintScreen > 0) ?
                                        (totalDrawBatchDoodad / totalPaintScreen) * 100.0 : 0.0;
        double drawRibbonPercent = (totalPaintScreen > 0) ?
                                   (totalDrawRibbon / totalPaintScreen) * 100.0 : 0.0;
        double drawCallbackPercent = (totalPaintScreen > 0) ?
                                     (totalDrawCallback / totalPaintScreen) * 100.0 : 0.0;
        double cM2SceneRenderDrawPercent = (totalPaintScreen > 0) ?
                                           (totalCM2SceneRenderDraw / totalPaintScreen) * 100.0 : 0.0;
        double frameOnLayerUpdatePercent = (totalPaintScreen > 0) ?
                                           (totalFrameOnLayerUpdate / totalPaintScreen) * 100.0 : 0.0;
        double cWorldRenderPercent = (totalPaintScreen > 0) ?
                                     (totalCWorldRender / totalPaintScreen) * 100.0 : 0.0;
        double cWorldUpdatePercent = (totalPaintScreen > 0) ?
                                     (totalCWorldUpdate / totalPaintScreen) * 100.0 : 0.0;
        double cWorldSceneRenderPercent = (totalPaintScreen > 0) ?
                                          (totalCWorldSceneRender / totalPaintScreen) * 100.0 : 0.0;
        double cWorldUnknownRenderPercent = (totalPaintScreen > 0) ?
                                            (totalCWorldUnknownRender / totalPaintScreen) * 100.0 : 0.0;
        double timeBetweenRenderPercent = (totalPaintScreen > 0) ?
                                          (totalTimeBetweenRender / totalPaintScreen) * 100.0 : 0.0;
        double frameOnScriptEventPercent = (totalPaintScreen > 0) ?
                                           (totalFrameOnScriptEvent / totalPaintScreen) * 100.0 : 0.0;

        // Convert microseconds to milliseconds for display
        double totalOnWorldRenderMs = totalOnWorldRender / 1000.0;
        double totalOnWorldUpdateMs = totalOnWorldUpdate / 1000.0;
        double totalCWorldRenderMs = totalCWorldRender / 1000.0;
        double totalCWorldUpdateMs = totalCWorldUpdate / 1000.0;
        double totalCWorldSceneRenderMs = totalCWorldSceneRender / 1000.0;
        double totalCWorldUnknownRenderMs = totalCWorldUnknownRender / 1000.0;
        double totalTimeBetweenRenderMs = totalTimeBetweenRender / 1000.0;
        double totalSpellVisualsRenderMs = totalSpellVisualsRender / 1000.0;
        double totalSpellVisualsTickMs = totalSpellVisualsTick / 1000.0;
        double totalUnitUpdateMs = totalUnitUpdate / 1000.0;
        double totalObjectUpdateHandlerMs = totalObjectUpdateHandler / 1000.0;
        double totalPlaySpellVisualMs = totalPlaySpellVisual / 1000.0;
        double totalUnknownOnRender1Ms = totalUnknownOnRender1 / 1000.0;
        double totalUnknownOnRender2Ms = totalUnknownOnRender2 / 1000.0;
        double totalUnknownOnRender3Ms = totalUnknownOnRender3 / 1000.0;
        double totalCM2SceneAdvanceTimeMs = totalCM2SceneAdvanceTime / 1000.0;
        double totalCM2SceneAnimateMs = totalCM2SceneAnimate / 1000.0;
        double totalCM2SceneDrawMs = totalCM2SceneDraw / 1000.0;
        double totalDrawBatchProjMs = totalDrawBatchProj / 1000.0;
        double totalDrawBatchMs = totalDrawBatch / 1000.0;
        double totalDrawBatchDoodadMs = totalDrawBatchDoodad / 1000.0;
        double totalDrawRibbonMs = totalDrawRibbon / 1000.0;
        double totalDrawParticleMs = totalDrawParticle / 1000.0;
        double totalDrawCallbackMs = totalDrawCallback / 1000.0;
        double totalCM2SceneRenderDrawMs = totalCM2SceneRenderDraw / 1000.0;
        double totalFrameOnLayerUpdateMs = totalFrameOnLayerUpdate / 1000.0;
        double totalFrameOnScriptEventMs = totalFrameOnScriptEvent / 1000.0;
        double totalEvtPaintMs = totalEvtPaint / 1000.0;
        double totalEvtIdleMs = totalEvtIdle / 1000.0;
        double totalCSimpleTopOnLayerUpdateMs = totalCSimpleTopOnLayerUpdate / 1000.0;
        double totalCSimpleTopOnLayerRenderMs = totalCSimpleTopOnLayerRender / 1000.0;
        double totalCSimpleFrameOnFrameRender1Ms = totalCSimpleFrameOnFrameRender1 / 1000.0;
        double totalCSimpleFrameOnFrameRender2Ms = totalCSimpleFrameOnFrameRender2 / 1000.0;
        double totalCSimpleModelOnFrameRenderMs = totalCSimpleModelOnFrameRender / 1000.0;

        // --- SUMMARY ---
        DEBUG_LOG(
                "--------------------------------------------------------------------------------------------------------------------------------------");
        // Calculate start time by subtracting 30 seconds from current time
        auto now = std::chrono::system_clock::now();
        auto thirtySecondsAgo = now - std::chrono::seconds(30);
        std::time_t start_time_t = std::chrono::system_clock::to_time_t(thirtySecondsAgo);
        std::time_t end_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm start_tm, end_tm;
#ifdef _WIN32
        localtime_s(&start_tm, &start_time_t);
        localtime_s(&end_tm, &end_time_t);
#else
        localtime_r(&start_time_t, &start_tm);
        localtime_r(&end_time_t, &end_tm);
#endif

        std::ostringstream start_oss, end_oss;
        start_oss << std::put_time(&start_tm, "%m-%d %H:%M:%S");
        end_oss << std::put_time(&end_tm, "%m-%d %H:%M:%S");

        DEBUG_LOG("--- STATS from " << start_oss.str() << " to " << end_oss.str() << " ---");
        NEWLINE_LOG();

        DEBUG_LOG("--- Main loop event times ---");
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(45) << "Total poll(networking) event time:"
               << std::right << std::setw(8) << totalEvtPoll / 1000.0 << " ms";
            DEBUG_LOG(ss.str());

            // Add ObjectUpdateHandler as a sub-item of networking
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(45) << "  ObjectUpdateHandler:"
               << std::right << std::setw(6) << objectUpdateHandlerPercent << "% ("
               << std::right << std::setw(8) << totalObjectUpdateHandlerMs << " ms)";
            DEBUG_LOG(ss.str());
        }
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(45) << "Total idle(unit movement) event time:"
               << std::right << std::setw(8) << totalEvtIdle / 1000.0 << " ms";
            DEBUG_LOG(ss.str());
        }
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(45) << "Total paint(render) event time:"
               << std::right << std::setw(8) << totalEvtPaint / 1000.0 << " ms";
            DEBUG_LOG(ss.str());
        }

        NEWLINE_LOG();

        // Convert PaintScreen to milliseconds
        double totalPaintScreenMs = totalPaintScreen / 1000.0;

        DEBUG_LOG(
                std::fixed << std::setprecision(2)
                           << "[Total] Render: " << std::right << std::setw(8) << totalPaintScreenMs
                           << " ms.  Frames: " << std::right << std::setw(6) << gPaintScreenStats.callCount
                           << ".  Time per frame: " << std::right << std::setw(6)
                           << (gPaintScreenStats.callCount > 0 ? totalPaintScreenMs /
                                                                 gPaintScreenStats.callCount
                                                               : 0.0)
                           << " ms.  Avg fps: "
                           << std::right << std::setw(6)
                           << (callCount > 0 ? callCount / (STATS_OUTPUT_INTERVAL_MS / 1000.0) : 0.0));

        {
            DEBUG_LOG("--- FUNCTION STATS (% OF TOTAL RENDER) ---");

            std::vector<std::pair<double, std::string>> allStats;
            std::stringstream ss;

            // OnWorldRender group - show parent then sorted sub-functions
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "OnWorldRender:"
               << std::right << std::setw(6) << onWorldRenderPercent << "% ("
               << std::right << std::setw(8) << totalOnWorldRenderMs << " ms)";
            DEBUG_LOG(ss.str());

            // OnWorldRender sub-functions sorted by performance
            std::vector<std::pair<double, std::string>> renderStats;

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  UnknownOnRender1:"
               << std::right << std::setw(6) << unknownOnRender1Percent << "% ("
               << std::right << std::setw(8) << totalUnknownOnRender1Ms << " ms)";
            renderStats.emplace_back(unknownOnRender1Percent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  UnknownOnRender2:"
               << std::right << std::setw(6) << unknownOnRender2Percent << "% ("
               << std::right << std::setw(8) << totalUnknownOnRender2Ms << " ms)";
            renderStats.emplace_back(unknownOnRender2Percent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  UnknownOnRender3:"
               << std::right << std::setw(6) << unknownOnRender3Percent << "% ("
               << std::right << std::setw(8) << totalUnknownOnRender3Ms << " ms)";
            renderStats.emplace_back(unknownOnRender3Percent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CM2Scene::AdvanceTime:"
               << std::right << std::setw(6) << cM2SceneAdvanceTimePercent << "% ("
               << std::right << std::setw(8) << totalCM2SceneAdvanceTimeMs << " ms)";
            renderStats.emplace_back(cM2SceneAdvanceTimePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CWorldSceneRender:"
               << std::right << std::setw(6) << cWorldSceneRenderPercent << "% ("
               << std::right << std::setw(8) << totalCWorldSceneRenderMs << " ms)";
            renderStats.emplace_back(cWorldSceneRenderPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CWorldRender:"
               << std::right << std::setw(6) << cWorldRenderPercent << "% ("
               << std::right << std::setw(8) << totalCWorldRenderMs << " ms)";
            renderStats.emplace_back(cWorldRenderPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CM2Scene::Animate:"
               << std::right << std::setw(6) << cM2SceneAnimatePercent << "% ("
               << std::right << std::setw(8) << totalCM2SceneAnimateMs << " ms)";
            renderStats.emplace_back(cM2SceneAnimatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CM2Scene::Draw:"
               << std::right << std::setw(6) << cM2SceneDrawPercent << "% ("
               << std::right << std::setw(8) << totalCM2SceneDrawMs << " ms)";
            renderStats.emplace_back(cM2SceneDrawPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  DrawBatchProj:"
               << std::right << std::setw(6) << drawBatchProjPercent << "% ("
               << std::right << std::setw(8) << totalDrawBatchProjMs << " ms)";
            renderStats.emplace_back(drawBatchProjPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  DrawBatch:"
               << std::right << std::setw(6) << drawBatchPercent << "% ("
               << std::right << std::setw(8) << totalDrawBatchMs << " ms)";
            renderStats.emplace_back(drawBatchPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  DrawBatchDoodad:"
               << std::right << std::setw(6) << drawBatchDoodadPercent << "% ("
               << std::right << std::setw(8) << totalDrawBatchDoodadMs << " ms)";
            renderStats.emplace_back(drawBatchDoodadPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  DrawRibbon:"
               << std::right << std::setw(6) << drawRibbonPercent << "% ("
               << std::right << std::setw(8) << totalDrawRibbonMs << " ms)";
            renderStats.emplace_back(drawRibbonPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  DrawCallback:"
               << std::right << std::setw(6) << drawCallbackPercent << "% ("
               << std::right << std::setw(8) << totalDrawCallbackMs << " ms)";
            renderStats.emplace_back(drawCallbackPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CM2SceneRender::Draw:"
               << std::right << std::setw(6) << cM2SceneRenderDrawPercent << "% ("
               << std::right << std::setw(8) << totalCM2SceneRenderDrawMs << " ms)";
            renderStats.emplace_back(cM2SceneRenderDrawPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  SpellVisualsTick:"
               << std::right << std::setw(6) << spellVisualsTickPercent << "% ("
               << std::right << std::setw(8) << totalSpellVisualsTickMs << " ms)";
            renderStats.emplace_back(spellVisualsTickPercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  SpellVisualsRender:"
               << std::right << std::setw(6) << spellVisualsRenderPercent << "% ("
               << std::right << std::setw(8) << totalSpellVisualsRenderMs << " ms)";
            renderStats.emplace_back(spellVisualsRenderPercent, ss.str());

            // Sort and display OnWorldRender sub-functions
            std::sort(renderStats.rbegin(), renderStats.rend());
            for (const auto &stat: renderStats) {
                DEBUG_LOG(stat.second);
            }

            // OnWorldUpdate group - show parent then sorted sub-functions
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "OnWorldUpdate:"
               << std::right << std::setw(6) << onWorldUpdatePercent << "% ("
               << std::right << std::setw(8) << totalOnWorldUpdateMs << " ms)";
            DEBUG_LOG(ss.str());

            // OnWorldUpdate sub-functions sorted by performance
            std::vector<std::pair<double, std::string>> updateStats;

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  CWorldUpdate:"
               << std::right << std::setw(6) << cWorldUpdatePercent << "% ("
               << std::right << std::setw(8) << totalCWorldUpdateMs << " ms)";
            updateStats.emplace_back(cWorldUpdatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "  UnitUpdate:"
               << std::right << std::setw(6) << unitUpdatePercent << "% ("
               << std::right << std::setw(8) << totalUnitUpdateMs << " ms)";
            updateStats.emplace_back(unitUpdatePercent, ss.str());

            // Sort and display OnWorldUpdate sub-functions
            std::sort(updateStats.rbegin(), updateStats.rend());
            for (const auto &stat: updateStats) {
                DEBUG_LOG(stat.second);
            }

            DEBUG_LOG("------");

            // Now add remaining stats to be sorted at top level
            allStats.clear();

//            ss.str("");
//            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "PlaySpellVisual:"
//               << std::right << std::setw(6) << playSpellVisualPercent << "% ("
//               << std::right << std::setw(8) << totalPlaySpellVisualMs << " ms)";
//            allStats.emplace_back(playSpellVisualPercent, ss.str());

            // Frame stats
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "All OnUpdates:"
               << std::right << std::setw(6) << frameOnLayerUpdatePercent << "% ("
               << std::right << std::setw(8) << totalFrameOnLayerUpdateMs << " ms)";
            allStats.emplace_back(frameOnLayerUpdatePercent, ss.str());

            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "All Events:"
               << std::right << std::setw(6) << frameOnScriptEventPercent << "% ("
               << std::right << std::setw(8) << totalFrameOnScriptEventMs << " ms)";
            allStats.emplace_back(frameOnScriptEventPercent, ss.str());

            double cSimpleTopOnLayerUpdatePercent = (totalPaintScreen > 0 ? (totalCSimpleTopOnLayerUpdate /
                                                                             totalPaintScreen * 100.0) : 0.0);
            ss.str("");
            ss << std::fixed << std::setprecision(2) << std::left << std::setw(25) << "UIParent OnUpdate:"
               << std::right << std::setw(6) << cSimpleTopOnLayerUpdatePercent << "% ("
               << std::right << std::setw(8) << totalCSimpleTopOnLayerUpdateMs << " ms)";
            allStats.emplace_back(cSimpleTopOnLayerUpdatePercent, ss.str());

            std::sort(allStats.rbegin(), allStats.rend());

            for (const auto &stat: allStats) {
                DEBUG_LOG(stat.second);
            }
        }
        NEWLINE_LOG();

        // --- DETAILED STATS ---
        DEBUG_LOG("--- DETAILED STATS ---");
        gPaintScreenStats.outputStats();
        gCSimpleTopOnLayerRenderStats.outputStats();
        gCSimpleTopOnLayerUpdateStats.outputStats();
        NEWLINE_LOG();
        gOnWorldRenderStats.outputStats();
        gUnknownOnRender1Stats.outputStats();
        gUnknownOnRender2Stats.outputStats();
        gUnknownOnRender3Stats.outputStats();
        gCM2SceneAdvanceTimeStats.outputStats();
        gCM2SceneAnimateStats.outputStats();
        gCM2ModelAnimateMTStats.outputStats();
        gCM2SceneDrawStats.outputStats();
        gDrawBatchProjStats.outputStats();
        gDrawBatchStats.outputStats();
        gDrawBatchDoodadStats.outputStats();
        gDrawRibbonStats.outputStats();
        gDrawParticleStats.outputStats();
        gDrawCallbackStats.outputStats();
        gCM2SceneRenderDrawStats.outputStats();
        gCWorldSceneRenderStats.outputStats();
        NEWLINE_LOG();
        gOnWorldUpdateStats.outputStats();
        gUnitUpdateStats.outputStats();
        gCWorldUpdateStats.outputStats();

        NEWLINE_LOG();
        gObjectUpdateHandlerStats.outputStats();
//        gPlaySpellVisualStats.outputStats();
        gFrameOnScriptEventStats.outputStats();
        gFrameOnLayerUpdateStats.outputStats();
        NEWLINE_LOG();

        gLuaCCollectgarbageStats.outputStats();
        gObjectFreeStats.outputStats();

        NEWLINE_LOG();

        // --- ADDON ONUPDATE PERFORMANCE ---
        if (!gAddonOnUpdateStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME ONUPDATE PERFORMANCE (min 1ms total)---");

            // Sort addons by total time
            std::vector<std::pair<double, std::string>> addonOnUpdateStats;
            for (auto it = gAddonOnUpdateStats.begin();
                 it != gAddonOnUpdateStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalTime >= 1000.0) {
                    addonOnUpdateStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(addonOnUpdateStats.rbegin(), addonOnUpdateStats.rend());

            for (auto it = addonOnUpdateStats.begin();
                 it != addonOnUpdateStats.end(); ++it) {
                gAddonOnUpdateStats[it->second].outputStats();
            }
        }

        // --- ADDON ONUPDATE MEMORY USAGE ---
        if (!gAddonOnUpdateMemoryStats.empty()) {
            DEBUG_LOG("--- ADDON ONUPDATE MEMORY USAGE (min 1KB total increase) ---");

            // Sort addons by total memory increase
            std::vector<std::pair<long long, std::string>> addonMemoryStats;
            for (auto it = gAddonOnUpdateMemoryStats.begin();
                 it != gAddonOnUpdateMemoryStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalMemoryIncrease >= 1) {
                    addonMemoryStats.push_back(std::make_pair(it->second.totalMemoryIncrease, it->first));
                }
            }
            std::sort(addonMemoryStats.rbegin(), addonMemoryStats.rend());

            for (auto it = addonMemoryStats.begin();
                 it != addonMemoryStats.end(); ++it) {
                gAddonOnUpdateMemoryStats[it->second].outputStats();
            }
        }

        // --- ADDON ONEVENT MEMORY USAGE ---
        if (!gAddonOnEventMemoryStats.empty()) {
            DEBUG_LOG("--- ADDON ONEVENT MEMORY USAGE (min 1KB total increase) ---");

            // Sort addons by total memory increase
            std::vector<std::pair<long long, std::string>> addonEventMemoryStats;
            for (auto it = gAddonOnEventMemoryStats.begin();
                 it != gAddonOnEventMemoryStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalMemoryIncrease >= 1) {
                    addonEventMemoryStats.push_back(std::make_pair(it->second.totalMemoryIncrease, it->first));
                }
            }
            std::sort(addonEventMemoryStats.rbegin(), addonEventMemoryStats.rend());

            for (auto it = addonEventMemoryStats.begin();
                 it != addonEventMemoryStats.end(); ++it) {
                gAddonOnEventMemoryStats[it->second].outputStats();
            }
        }

        // --- ADDON EVENT STATS ---
        if (!gAddonScriptEventStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME EVENTS PERFORMANCE (min 1ms total)---");

            // Sort addons by total time
            std::vector<std::pair<double, std::string>> addonStats;
            for (auto it = gAddonScriptEventStats.begin();
                 it != gAddonScriptEventStats.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalTime >= 1000.0) {
                    addonStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(addonStats.rbegin(), addonStats.rend());

            for (auto it = addonStats.begin();
                 it != addonStats.end(); ++it) {
                gAddonScriptEventStats[it->second].outputStats();
            }
        }

        // --- ADDON SLOWEST EVENTS REPORT ---
        if (!gAddonEventStats.empty()) {
            DEBUG_LOG("--- ADDON/FRAME SLOWEST EVENTS REPORT (min 1ms combined duration) ---");

            for (const auto &addonPair: gAddonEventStats) {
                const std::string &addonName = addonPair.first;
                std::vector<EventStats> slowEvents = addonPair.second; // Copy for sorting

                if (!slowEvents.empty()) {
                    // Calculate total duration for this addon
                    double totalAddonDuration = 0.0;
                    for (const auto &event: slowEvents) {
                        totalAddonDuration += event.duration;
                    }

                    // Skip if total duration is less than 1ms (1000 microseconds)
                    if (totalAddonDuration < 1000.0) {
                        continue;
                    }

                    // Sort events by duration (descending) only when displaying
                    std::sort(slowEvents.begin(), slowEvents.end());

                    // Only show top 10 slowest events
                    size_t eventsToShow = slowEvents.size() < 10 ? slowEvents.size() : 10;
                    DEBUG_LOG("[" << addonName << "] Top " << eventsToShow << " slowest events (out of "
                                  << slowEvents.size() << " total):");

                    for (size_t i = 0; i < eventsToShow; ++i) {
                        const EventStats &event = slowEvents[i];
                        DEBUG_LOG("  " << std::right << std::setw(2) << (i + 1) << ".  "
                                       << std::left << std::setw(50) << GetEventName(event.eventCode)
                                       << " Total Duration: " << std::right << std::setw(8) << std::fixed
                                       << std::setprecision(3)
                                       << event.duration / 1000.0 << " ms"
                                       << "      Count: " << std::right << std::setw(6) << event.count);
                    }
                }
            }
        }


        // --- SPELL VISUAL PERFORMANCE (TOP 10 SLOWEST) ---
        if (!gSpellVisualStatsById.empty()) {
            DEBUG_LOG("--- SPELL VISUAL PERFORMANCE (min 1ms total) ---");

            // Sort spells by total time
            std::vector<std::pair<double, uint32_t>> spellStats;
            for (auto it = gSpellVisualStatsById.begin(); it != gSpellVisualStatsById.end(); ++it) {
                if (it->second.callCount > 0 && it->second.totalTime >= 1000.0) {
                    spellStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(spellStats.rbegin(), spellStats.rend());

            // Show only top 10 spells
            size_t spellsToShow = spellStats.size() < 10 ? spellStats.size() : 10;
            for (size_t i = 0; i < spellsToShow; ++i) {
                gSpellVisualStatsById[spellStats[i].second].outputStats(20);
            }
        }

        // --- EVENT CODE DURATION STATISTICS (TOP 10) ---
        if (!gEventCodeStats.empty()) {
            DEBUG_LOG("--- TOTAL EVENT DURATION STATISTICS (SHOULD INCLUDE ALL ADDONS) ---");

            // Sort event codes by total time
            std::vector<std::pair<double, int>> eventCodeStats;
            for (auto it = gEventCodeStats.begin(); it != gEventCodeStats.end(); ++it) {
                if (it->second.callCount > 0) {
                    eventCodeStats.push_back(std::make_pair(it->second.totalTime, it->first));
                }
            }
            std::sort(eventCodeStats.rbegin(), eventCodeStats.rend());

            // Show only top 10 events
            size_t eventsToShow = eventCodeStats.size() < 10 ? eventCodeStats.size() : 10;
            for (size_t i = 0; i < eventsToShow; ++i) {
                gEventCodeStats[eventCodeStats[i].second].outputStats(45);
            }
        }

        // Clear all stats
        gRenderWorldStats.clearStats();
        gOnWorldRenderStats.clearStats();
        gOnWorldUpdateStats.clearStats();
        gCWorldRenderStats.clearStats();
        gCWorldUpdateStats.clearStats();
        gCWorldSceneRenderStats.clearStats();
        gCWorldUnknownRenderStats.clearStats();
        gTimeBetweenRenderStats.clearStats();
        gFrameOnScriptEventStats.clearStats();
        gCSimpleFrameOnFrameRender1Stats.clearStats();
        gCSimpleFrameOnFrameRender2Stats.clearStats();
        gCSimpleModelOnFrameRenderStats.clearStats();
        gCSimpleTopOnLayerUpdateStats.clearStats();
        gCSimpleTopOnLayerRenderStats.clearStats();
        gSpellVisualsRenderStats.clearStats();
        gSpellVisualsTickStats.clearStats();
        gUnitUpdateStats.clearStats();
        gObjectUpdateHandlerStats.clearStats();
        gPlaySpellVisualStats.clearStats();
        gUnknownOnRender1Stats.clearStats();
        gUnknownOnRender2Stats.clearStats();
        gUnknownOnRender3Stats.clearStats();
        gCM2SceneAdvanceTimeStats.clearStats();
        gCM2SceneAnimateStats.clearStats();
        gCM2SceneDrawStats.clearStats();
        gPaintScreenStats.clearStats();
        gDrawBatchProjStats.clearStats();
        gDrawBatchStats.clearStats();
        gDrawBatchDoodadStats.clearStats();
        gDrawRibbonStats.clearStats();
        gDrawParticleStats.clearStats();
        gDrawCallbackStats.clearStats();
        gCM2SceneRenderDrawStats.clearStats();
        gFrameOnLayerUpdateStats.clearStats();
        gLuaCCollectgarbageStats.clearStats();
        gCM2ModelAnimateMTStats.clearStats();
        gObjectFreeStats.clearStats();

        // Clear addon stats
        for (auto it = gAddonScriptEventStats.begin();
             it != gAddonScriptEventStats.end(); ++it) {
            it->second.clearStats();
        }

        // Clear addon OnUpdate stats
        for (auto it = gAddonOnUpdateStats.begin();
             it != gAddonOnUpdateStats.end(); ++it) {
            it->second.clearStats();
        }

        // Clear addon memory stats
        for (auto it = gAddonOnUpdateMemoryStats.begin();
             it != gAddonOnUpdateMemoryStats.end(); ++it) {
            it->second.clearStats();
        }

        for (auto it = gAddonOnEventMemoryStats.begin();
             it != gAddonOnEventMemoryStats.end(); ++it) {
            it->second.clearStats();
        }


        // Clear addon event stats
        gAddonEventStats.clear();

        // Clear spell visual stats
        for (auto it = gSpellVisualStatsById.begin(); it != gSpellVisualStatsById.end(); ++it) {
            it->second.clearStats();
        }

        // Clear event code stats
        for (auto it = gEventCodeStats.begin(); it != gEventCodeStats.end(); ++it) {
            it->second.clearStats();
        }
        gEventCodeStartTimes.clear();

        // Clear total events stats
        gTotalEventsStats.clearStats();

        // Clear individual event stats
        for (auto it = gEventStats.begin(); it != gEventStats.end(); ++it) {
            it->second.clearStats();
        }

        DEBUG_LOG(
                "--------------------------------------------------------------------------------------------------------------------------------------");

        NEWLINE_LOG();
    }

}