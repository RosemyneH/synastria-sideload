#include <cfloat>
#include <cstdint>
#include "SRgn.h"
#include "Engine/Source/Storm/H/SCritSect.h"
#include "Engine/Source/Storm/H/stpl.h"
#include "Engine/Source/Storm/H/storm.h"

#define SOURCE_NEWRECT 0x1
#define SOURCE_OVERLAPPED 0x2
#define SOURCE_PARAMONLY 0x10000

// STATIC: 0x00CB7408
static TSExportTableSyncReuse<RGN, HSRGN, HLOCKEDRGN, CCritSect> s_rgntable;

// VERIFY: 6020b8a4 manual
// OFFSET: 0x00776070
static int32_t IsNullRect(const RECTF* rect) {
    return rect->left >= rect->right || rect->bottom >= rect->top;
}

// INLINED
static int32_t CheckForIntersection(const RECTF* sourcerect, const RECTF* targetrect) {
    return sourcerect->left < targetrect->right && sourcerect->bottom < targetrect->top && sourcerect->right > targetrect->left && sourcerect->top > targetrect->bottom;
}

// INLINED
static int32_t CompareRects(const RECTF* rect1, const RECTF* rect2) {
    return rect1->left == rect2->left && rect1->bottom == rect2->bottom && rect1->right == rect2->right && rect1->top == rect2->top;
}

// INLINED
static void DeleteRect(RECTF* rect) {
    rect->left = FLT_MAX;
    rect->bottom = FLT_MAX;
    rect->right = FLT_MAX;
    rect->top = FLT_MAX;
}

// INLINED
static void DeleteSourceRect(TSGrowableArray<_SOURCE>* sourcearray, uint32_t index) {
    _SOURCE* source = &(*sourcearray)[index];
    DeleteRect(&source->rect);
    source->param = nullptr;
    source->sequence = -1;
    source->flags = 0;
}

// VERIFY: 60491716 manual
// OFFSET: 0x00776140
static void ProcessBooleanOperation(TSGrowableArray<_SOURCE>* sourcearray, int32_t combinemode) {
    for (uint32_t i = 0; i < sourcearray->Count(); i++) {
        _SOURCE* source = &(*sourcearray)[i];

        switch (combinemode) {
            case SRGN_AND:
                if (!(source->flags & SOURCE_OVERLAPPED)) {
                    DeleteSourceRect(sourcearray, i);
                }
                break;
            case SRGN_XOR:
                if (source->flags & SOURCE_OVERLAPPED) {
                    DeleteSourceRect(sourcearray, i);
                }
                break;
            case SRGN_DIFF:
                if (source->flags & (SOURCE_NEWRECT | SOURCE_OVERLAPPED)) {
                    DeleteSourceRect(sourcearray, i);
                }
                break;
            case SRGN_COPY:
                if (source->flags & SOURCE_NEWRECT) {
                    DeleteSourceRect(sourcearray, i);
                }
                break;
            default:
                break;
        }

        source->flags = 0;
    }
}

// VERIFY: e160523f manual
// OFFSET: 0x00776260
static void AddSourceRect(TSGrowableArray<_SOURCE>* sourcearray, const RECTF* rect, void* param, int32_t sequence, uint32_t flags) {
    _SOURCE* source = sourcearray->New();
    source->rect = *rect;
    source->param = param;
    source->sequence = sequence;
    source->flags = flags;
}

// VERIFY: 08c9fdeb manual
// OFFSET: 0x007762A0
static void FragmentSourceRectangles(TSGrowableArray<_SOURCE>* sourcearray, uint32_t firstindex, uint32_t lastindex, int32_t previousoverlap, const RECTF* rect, void* param, int32_t sequence) {
    int32_t overlapsexisting = previousoverlap;

    for (uint32_t index = firstindex; index < lastindex; index++) {
        const RECTF* checkrect = &(*sourcearray)[index].rect;

        if (!CheckForIntersection(checkrect, rect)) {
            continue;
        }

        if (CompareRects(checkrect, rect)) {
            (*sourcearray)[index].flags |= SOURCE_OVERLAPPED;
            overlapsexisting = 1;
            continue;
        }

        const RECTF* overlaprect[2] = { rect, checkrect };
        int32_t minleft = checkrect->left < rect->left;
        int32_t maxleft = rect->left < checkrect->left;
        int32_t minbottom = checkrect->bottom < rect->bottom;
        int32_t maxbottom = rect->bottom < checkrect->bottom;
        int32_t minright = checkrect->right < rect->right;
        int32_t maxright = rect->right < checkrect->right;
        int32_t mintop = checkrect->top < rect->top;
        int32_t maxtop = rect->top < checkrect->top;

        RECTF newrect[5];
        newrect[0].left = overlaprect[minbottom]->left;
        newrect[0].bottom = overlaprect[minbottom]->bottom;
        newrect[0].right = overlaprect[minbottom]->right;
        newrect[0].top = overlaprect[maxbottom]->bottom;
        newrect[1].left = overlaprect[maxtop]->left;
        newrect[1].bottom = overlaprect[mintop]->top;
        newrect[1].right = overlaprect[maxtop]->right;
        newrect[1].top = overlaprect[maxtop]->top;
        newrect[2].left = overlaprect[minleft]->left;
        newrect[2].bottom = overlaprect[maxbottom]->bottom;
        newrect[2].right = overlaprect[maxleft]->left;
        newrect[2].top = overlaprect[mintop]->top;
        newrect[3].left = overlaprect[minright]->right;
        newrect[3].bottom = overlaprect[maxbottom]->bottom;
        newrect[3].right = overlaprect[maxright]->right;
        newrect[3].top = overlaprect[mintop]->top;
        newrect[4].left = overlaprect[maxleft]->left;
        newrect[4].bottom = overlaprect[maxbottom]->bottom;
        newrect[4].right = overlaprect[minright]->right;
        newrect[4].top = overlaprect[mintop]->top;

        int32_t overlaps[5][2];

        for (int32_t i = 0; i < 5; i++) {
            if (IsNullRect(&newrect[i])) {
                overlaps[i][0] = 0;
                overlaps[i][1] = 0;
            } else {
                overlaps[i][0] = CheckForIntersection(rect, &newrect[i]);
                overlaps[i][1] = CheckForIntersection(checkrect, &newrect[i]);
            }
        }

        for (int32_t i = 0; i < 5; i++) {
            if (overlaps[i][0]) {
                FragmentSourceRectangles(sourcearray, index + 1, lastindex, overlapsexisting || overlaps[i][1], &newrect[i], param, sequence);
            }

            if (overlaps[i][1]) {
                _SOURCE* source = &(*sourcearray)[index];
                AddSourceRect(sourcearray, &newrect[i], source->param, source->sequence, (source->flags & ~(SOURCE_NEWRECT | SOURCE_OVERLAPPED)) | (overlaps[i][0] ? SOURCE_OVERLAPPED : 0));
            }
        }

        DeleteSourceRect(sourcearray, index);
        return;
    }

    AddSourceRect(sourcearray, rect, param, sequence, (overlapsexisting ? SOURCE_OVERLAPPED : 0) | SOURCE_NEWRECT);
}

// VERIFY: c9c28f6e manual
// OFFSET: 0x007767C0
static void OptimizeSource(TSGrowableArray<_SOURCE>* sourcearray) {
    uint32_t index = 0;

    while (index < sourcearray->Count()) {
        if (IsNullRect(&(*sourcearray)[index].rect)) {
            uint32_t count = sourcearray->Count() - 1;
            (*sourcearray)[index] = (*sourcearray)[count];
            sourcearray->SetCount(count);
        } else {
            index++;
        }
    }
}

// INLINED
static void InvalidateRegion(RGN* rgn) {
    rgn->dirty = 1;
    DeleteRect(&rgn->foundparamsrect);
}

// INLINED
static void ClearRegion(RGN* rgn) {
    rgn->source.SetCount(0);
    rgn->combined.SetCount(0);
    rgn->foundparams.SetCount(0);
    DeleteRect(&rgn->foundparamsrect);
    rgn->sequence = 0;
    rgn->dirty = 0;
}

// VERIFY: ddb70c17 auto
// OFFSET: 0x00776900
RGN::~RGN() {
}

// VERIFY: ece264b9 manual
// OFFSET: 0x00776AB0
void STORMAPI SRgnDestroy() {
    s_rgntable.Destroy();
}

// VERIFY: 182f77be manual
// OFFSET: 0x00777420
void STORMAPI SRgnCombineRectf(HSRGN handle, const RECTF* rect, void* param, int32_t combinemode) {
    if (!handle) {
        DebugValidate("handle");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    if (!rect) {
        DebugValidate("rect");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    if (combinemode < SRGN_AND) {
        DebugValidate("combinemode >= 1");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    if (combinemode > SRGN_PARAMONLY) {
        DebugValidate("combinemode <= 6");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    HLOCKEDRGN lockedhandle;
    RGN* rgn = s_rgntable.Lock(handle, &lockedhandle, 0);

    if (!rgn) {
        return;
    }

    if (combinemode == SRGN_OR || combinemode == SRGN_PARAMONLY) {
        if (!IsNullRect(rect)) {
            AddSourceRect(&rgn->source, rect, param, ++rgn->sequence, combinemode == SRGN_PARAMONLY ? SOURCE_PARAMONLY : 0);
        }
    } else {
        if (!IsNullRect(rect)) {
            FragmentSourceRectangles(&rgn->source, 0, rgn->source.Count(), 0, rect, param, ++rgn->sequence);
        }

        ProcessBooleanOperation(&rgn->source, combinemode);
        OptimizeSource(&rgn->source);
    }

    InvalidateRegion(rgn);
    s_rgntable.Unlock(lockedhandle);
}

// VERIFY: 31d5dafb manual
// OFFSET: 0x00777590
void STORMAPI SRgnGetBoundingRectf(HSRGN handle, RECTF* rect) {
    if (!handle) {
        DebugValidate("handle");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    if (!rect) {
        DebugValidate("rect");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    rect->left = FLT_MAX;
    rect->bottom = FLT_MAX;
    rect->right = FLT_MIN;
    rect->top = FLT_MIN;

    HLOCKEDRGN lockedhandle;
    RGN* rgn = s_rgntable.Lock(handle, &lockedhandle, 0);

    if (!rgn) {
        return;
    }

    const _SOURCE* source = rgn->source.Ptr();
    uint32_t count = rgn->source.Count();

    for (uint32_t i = 0; i < count; i++) {
        if (source[i].flags & SOURCE_PARAMONLY) {
            continue;
        }

        rect->left = source[i].rect.left <= rect->left ? source[i].rect.left : rect->left;
        rect->bottom = source[i].rect.bottom <= rect->bottom ? source[i].rect.bottom : rect->bottom;
        rect->right = source[i].rect.right >= rect->right ? source[i].rect.right : rect->right;
        rect->top = source[i].rect.top >= rect->top ? source[i].rect.top : rect->top;
    }

    s_rgntable.Unlock(lockedhandle);

    if (IsNullRect(rect)) {
        rect->left = 0.0f;
        rect->bottom = 0.0f;
        rect->right = 0.0f;
        rect->top = 0.0f;
    }
}

// VERIFY: 04309b8a manual
// OFFSET: 0x00777940
void STORMAPI SRgnDelete(HSRGN handle) {
    if (!handle) {
        DebugValidate("handle");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    s_rgntable.Delete(handle);
}

// VERIFY: 3c5dae2f manual
// OFFSET: 0x00777980
void STORMAPI SRgnCreate(HSRGN* handle, uint32_t reserved) {
    if (!handle) {
        DebugValidate("handle");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    *handle = nullptr;

    if (reserved) {
        DebugValidate("!reserved");
        SErrSetLastError(STORM_ERROR_INVALID_PARAMETER);
        return;
    }

    HLOCKEDRGN lockedhandle;
    RGN* rgn = s_rgntable.NewLock(handle, &lockedhandle);
    ClearRegion(rgn);
    s_rgntable.Unlock(lockedhandle);
}
