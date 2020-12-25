#include "processor.h"

void processor::LoadSegmentSelectors(segmentSelector* segment_selector)
{
    segment_selector->es = GetEsSelector();
    segment_selector->cs = GetCsSelector();
    segment_selector->ss = GetSsSelector();
    segment_selector->ds = GetDsSelector();
    segment_selector->fs = GetFsSelector();
    segment_selector->gs = GetGsSelector();
    segment_selector->ldtr = GetLdtrSelector();
    segment_selector->tr = GetTrSelector();
}
