#include "processor.h"
#include "print.h"

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

void processor::ReadGDTEntry(GDTR gdt, UINT16 selector, vmxGdtEntry* entry)
{
    processor::GDTEntry* tmp_entry = nullptr;

    // Zero out entry
    entry->fields.access = 0;
    entry->fields.limit = 0;
    entry->fields.base = 0;

    if ((selector & 3) == 1) {
        // LDT
        MDbgPrint("Failed getting gdt entry base. LDT implementation needed.\n");
        return;
    }


   tmp_entry = reinterpret_cast<processor::GDTEntry*>(gdt.base + (selector >> kRplMask) * sizeof(processor::GDTEntry));

   entry->fields.limit = __segmentlimit(selector);
   entry->fields.access = __lar(selector);
   entry->fields.base = ((tmp_entry->base2 << 24) | (tmp_entry->base1 << 16) | (tmp_entry->base0)) & MAXULONG;
}
