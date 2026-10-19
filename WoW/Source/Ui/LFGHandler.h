#ifndef WOW_SOURCE_UI_LFGHANDLER_H
#define WOW_SOURCE_UI_LFGHANDLER_H

class CDataStore;
class LFGLIST;
class LFGListParty;
class LFGListPlayer;

namespace LFGHandler {
    void ReadGroupInfo(CDataStore* msg, LFGListParty* party);
    void ReadPlayerInfo(CDataStore* msg, LFGLIST* list, LFGListPlayer* player);
}

#endif
