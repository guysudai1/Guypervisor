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


   tmp_entry = reinterpret_cast<processor::GDTEntry*>((uintptr_t)gdt.base + (selector & ~kRplMask));

   entry->fields.limit = __segmentlimit(selector);
   entry->fields.access = __lar(selector) >> 8;
   entry->fields.base = ((tmp_entry->base2 << 24) | (tmp_entry->base1 << 16) | (tmp_entry->base0)) & MAXULONG;

   if (tmp_entry->system == 0) {
       // MDbgPrint("[BEFORE] Translated system GDT entry: %X\n", entry->fields.base);
       entry->fields.base |= (((UINT64)tmp_entry->base3) << 32);
       // MDbgPrint("[AFTER] Translated system GDT entry: %X\n", entry->fields.base);
   }
}
