#include "stdafx.h"
#include "tintin.h"
#include "resource.h"
#include "telnet.h"
#include "ttcoreex.h"
#include "Utils.h"
#include "Logs.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

using namespace std;

extern GET_WNDSIZE_FUNC GetWindowSize;

static unsigned long 
	Automap_MaxIdenticalExits = 5,
	Automap_MaxDifferentExits = 1;

typedef enum
{
	RenderTiny		= 1, /* 1x1 */
	RenderNormal	= 2, /* 5x3 */
	RenderFull		= 3  /* 7x5 */
} RenderStyle;

struct MapRoomFlag
{
	wstring			Name;
	wstring			Signature;
	wstring			Color;
};

static map<wstring, MapRoomFlag> RoomFlags;

struct MapDirection
{
	wstring			Command;
	wstring			Reversed;
	int				dX, dY, dZ;
	wstring			Signature;
};

struct RoomID
{
	unsigned long	VNum;  // actual VNum (from MSDP/GMCP/...) or hash-code of name + descr
	unsigned long	Index; // subindex for rooms with identical VNums
	
	bool operator == (const RoomID &other) const { return (VNum == other.VNum) && (Index == other.Index); };
	bool operator < (const RoomID &other) const { return (VNum < other.VNum) || ((VNum == other.VNum) && (Index < other.Index)); };

	wstring toString() const
	{ 
		wchar_t buf[32]; 
		if (Index)
			swprintf(buf, L"%u.%u", VNum, Index); 
		else
			swprintf(buf, L"%u", VNum); 
		return wstring(buf); 
	};

	static bool fromString(const wchar_t *Source, RoomID &Output)
	{
		unsigned long num, ind;
		int ret = swscanf(Source, L"%u.%u", &num, &ind);
		if (ret == 2) {
			Output.VNum = num;
			Output.Index = ind;
			return true;
		}
		if (ret == 1) {
			Output.VNum = num;
			Output.Index = 0;
			return true;
		}
		return false;
	};

	RoomID(unsigned long num = 0, unsigned long ind = 0): VNum(num), Index(ind) {};
};

static map<wstring, MapDirection> Directions;
static map<wstring, wstring> SynonymToDirection;
struct SetOfSynonyms {
	set<wstring> synonyms;
};
static map<wstring, SetOfSynonyms > DirectionToSynonyms;

class CMapExit
{
private:
	RoomID			m_From, m_To;
	wstring			m_Command;
	wstring			m_PassCommand;

public:
	void onRoomIDChanged(const RoomID &oldval, const RoomID &newval)
	{
		if (m_From == oldval)
			m_From = newval;
		if (m_To == oldval)
			m_To = newval;
	};
	RoomID toID() const { return m_To; };
	RoomID fromID() const { return m_From; };
	wstring command() const { return m_Command; };
	bool equals(const wstring &Command)
	{
		map<wstring, wstring>::const_iterator it_syn = SynonymToDirection.find(Command);
		if (it_syn != SynonymToDirection.end())
			return m_Command == it_syn->second;
		return m_Command == Command;
	};

	void update(const RoomID &From, const RoomID &To) 
	{
		m_From = From;
		m_To = To;
	};
	void setPassCommand(const wstring &PassCommand)
	{
		m_PassCommand = PassCommand;
	};
	wstring passCommand() const
	{
		return m_PassCommand;
	};
	wstring getAddCommand() const
	{
		wstring ret = L"";
		
		ret += cCommandChar;
		ret += L"mapper add exit ";
		ret += m_From.toString();
		ret += L" {" + m_Command + L"} ";
		ret += m_To.toString();

		return ret;
	};
	MapDirection direction() const
	{
		map<wstring, MapDirection>::iterator  it_dir = Directions.find(m_Command);
		if (it_dir != Directions.end())
			return it_dir->second;
		MapDirection ret;
		ret.Command = m_Command;
		ret.Reversed = L"";
		ret.dX = ret.dY = ret.dZ = 0;
		ret.Signature = L"";
		return ret;
	};

	bool operator < (const CMapExit &Other) const
	{
		return (m_From    == Other.fromID() ) ? 
			   (m_Command  < Other.command()) : 
		       (m_From     < Other.fromID() ) ;
	};
	bool operator == (const CMapExit &Other) const
	{
		return m_From == Other.fromID() && m_Command == Other.command();
	};

	CMapExit(const RoomID &From, const RoomID &To, const wstring &Command,
		const wstring &PassCommand = L"") :
	m_From(From), m_To(To), m_Command(Command), m_PassCommand(PassCommand)
	{
		map<wstring, wstring>::const_iterator it_syn = SynonymToDirection.find(Command);
		if (it_syn != SynonymToDirection.end())
			m_Command = it_syn->second;
		else
			m_Command = Command;
	};
};

class CMapRoom
{
private:
	RoomID			m_ID;
	
	wstring			m_Name;
	wstring			m_Descr;
	
	wstring			m_Area;
	wstring			m_Comment;

	bool			m_AutoID;
	bool			m_Duplicate;

	set<wstring>	m_Flags;
	set<CMapExit>	m_Entrances;
	set<CMapExit>	m_Exits;

	int				m_Avoidance;


public:
	RoomID ID() const;
	RoomID autoID(bool Set);
	
	void addEntrance(const RoomID &From, const wstring &Direction);
	void delEntrance(const RoomID &From, const wstring &Direction);

	bool addExit(const RoomID &To, const wstring &Direction);
	bool delExit(const wstring &Direction);
	bool addPassCommand(const wstring &Direction, const wstring &PassCommand);

	bool autoID() const;
	bool duplicate() const;
	wstring name() const;
	wstring descr() const;
	wstring area() const;
	wstring comment() const;
	int avoidance() const;

	void update(const wstring &Name, const wstring &Descr, const wstring &Area, const wstring &Comment = L"", const wstring &Flags = L"");

	void setComment(const wstring &Comment);
	void addFlag(const wstring &Flag);
	void delFlag(const wstring &Flag);
	void setAvoidance(int Avoidance);

	set<wstring> getFlags() const;

	wstring getAddCommand() const;

	set<CMapExit> entrances() const;
	set<CMapExit> exits() const;

	vector<unsigned long> allPaths(unsigned long MaxLength, unsigned long MaxChanges) const;

	CMapExit getExit(const wstring &Command) const;
	CMapExit getExit(unsigned long ToVNum) const;

	bool merge(CMapRoom *pOther, RoomID &ChangedRoomID);

	int trackMaxLength(const vector< pair<wstring, unsigned long> > &Path) const;

	vector<wstring> render(RenderStyle style, bool root, bool use_colors) const;

	void onRoomIDChanged(const RoomID &oldval, const RoomID &newval, bool WithNeighbours);

	bool removeFromList(const RoomID *pID = NULL) const;
	void addToList() const;

public:
	CMapRoom(const RoomID &ID, const wstring &Name, const wstring &Descr = L"", 
		const wstring &Area = L"", const wstring &Comment = L"", const wstring &Flags = L"");
	~CMapRoom();
};

static vector< pair<wstring, unsigned long> > TrackHistory;
static map<unsigned long, map<unsigned long, CMapRoom*> > Rooms; // Rooms[VNum][Index]
static map< RoomID, set<CMapExit> > DelayedEntrances;

static void add_delayed_entrance(const RoomID &From, const RoomID &To, const wstring &Command)
{
	map<RoomID, set<CMapExit> >::iterator it_entrance;
	it_entrance = DelayedEntrances.find(To);
	if (it_entrance == DelayedEntrances.end())
		DelayedEntrances[To] = set<CMapExit>();
	DelayedEntrances[To].insert(CMapExit(From, To, Command));
}
static void del_delayed_entrance(const RoomID &From, const RoomID &To, const wstring &Command)
{
	map<RoomID, set<CMapExit> >::iterator it_entrance;
	it_entrance = DelayedEntrances.find(To);
	if (it_entrance != DelayedEntrances.end())
	{
		set<CMapExit>::iterator it_exit = DelayedEntrances[To].find(CMapExit(From, To, Command));
		if (it_exit != DelayedEntrances[To].end())
			DelayedEntrances[To].erase(it_exit);
	}
}
static CMapRoom *findRoom(const RoomID &ID)
{
	map<unsigned long, map<unsigned long, CMapRoom*> >::const_iterator it_vnum;
	map<unsigned long, CMapRoom*>::const_iterator it_index;

	it_vnum = Rooms.find(ID.VNum);
	if (it_vnum == Rooms.end())
		return NULL;
	
	it_index = it_vnum->second.find(ID.Index);
	if (it_index == it_vnum->second.end()) {
		if (ID.Index == 0 && it_vnum->second.size() > 0)
			return it_vnum->second.begin()->second;
		return NULL;
	}

	return it_index->second;
}

bool CMapRoom::removeFromList(const RoomID *pID) const
{
	map<unsigned long, map<unsigned long, CMapRoom*> >::iterator it_vnum;
	map<unsigned long, CMapRoom*>::iterator it_index;

	RoomID id;
	if (pID)
		id = *pID;
	else
		id = m_ID;

	it_vnum = Rooms.find(id.VNum);
	if (it_vnum != Rooms.end()) {
		it_index = it_vnum->second.find(id.Index);
		if (it_index != it_vnum->second.end()) {
			it_vnum->second.erase(it_index);
			if (it_vnum->second.size() == 0)
				Rooms.erase(it_vnum);
			return true;
		}
	}

	set<CMapExit>::const_iterator it_exit;
	for (it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		CMapRoom *room = findRoom(it_exit->toID());
		if (room)
			room->delEntrance(m_ID, it_exit->command());
	}
	for (it_exit = m_Entrances.begin(); it_exit != m_Entrances.end(); it_exit++) {
		CMapRoom *room = findRoom(it_exit->fromID());
		if (room) {
			if (room->delExit(it_exit->command()) && room->autoID())
				room->autoID(true);
		}
	}

	return false;
}

void CMapRoom::addToList() const
{
	map<unsigned long, map<unsigned long, CMapRoom*> >::iterator it_vnum;
	it_vnum = Rooms.find(m_ID.VNum);
	if (it_vnum == Rooms.end())
		Rooms[m_ID.VNum] = map<unsigned long, CMapRoom*>();
	Rooms[m_ID.VNum][m_ID.Index] = (CMapRoom*)this;
}

void CMapRoom::onRoomIDChanged(const RoomID &oldval, const RoomID &newval, bool WithNeighbours)
{
	if (newval == oldval)
		return;

	set<CMapExit>::iterator it_exit;

	for (it_exit = m_Entrances.begin(); it_exit != m_Entrances.end(); it_exit++) {
		if (WithNeighbours) {
			CMapRoom *from = findRoom(it_exit->fromID());
			if (from)
				from->onRoomIDChanged(oldval, newval, false);
		}
		it_exit->onRoomIDChanged(oldval, newval);
	}
	for (it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (WithNeighbours) {
			CMapRoom *to = findRoom(it_exit->toID());
			if (to) {
				to->onRoomIDChanged(oldval, newval, false);
			} else {
				map< RoomID, set<CMapExit> >::iterator it_entrances;
				it_entrances = DelayedEntrances.find(it_exit->toID());
				if (it_entrances != DelayedEntrances.end()) {
					CMapExit oldexit = CMapExit(it_exit->fromID(), it_exit->toID(), it_exit->command());
					it_entrances->second.erase(oldexit);
					oldexit.onRoomIDChanged(oldval, newval);
					it_entrances->second.insert(oldexit);
				}
			}
		}
		it_exit->onRoomIDChanged(oldval, newval);
	}
}

static unsigned long hash(const wstring &str)
{
	unsigned long ret = 0x56EF7801;
	for (int i = 0; i < str.length(); i++)
		ret = ret * 4001 + str[i];
	return ret;
}

static unsigned long hash(const vector<unsigned long> &vals)
{
	unsigned long ret = 0x56EF7801;
	for (int i = 0; i < vals.size(); i++)
		ret = ret * 4001 + vals[i];
	return ret;
}

RoomID CMapRoom::ID() const
{
	return m_ID;
}

static set<RoomID> paths_seen;

vector<unsigned long> CMapRoom::allPaths(unsigned long MaxLength, unsigned long MaxChanges) const
{
	vector<unsigned long> ret;

	if (MaxLength > 0) {
		set<CMapExit>::const_iterator exit;

		ret.push_back(m_Exits.size());

		for (exit = m_Exits.begin(); exit != m_Exits.end(); exit++) {
			MapDirection dir = exit->direction();
			ret.push_back(hash(dir.Command));
			ret.push_back(exit->toID().VNum);

			if (paths_seen.find(exit->toID()) != paths_seen.end())
				continue;
			paths_seen.insert(exit->toID());

			if (exit->toID().VNum != m_ID.VNum && MaxChanges <= 1)
				continue;

			if (MaxLength > 1) {
				CMapRoom *room = findRoom(exit->toID());
				if (room) {
					unsigned long ch = MaxChanges;
					if (exit->toID().VNum != m_ID.VNum)
						ch -= 1;
					vector<unsigned long> subpaths = room->allPaths(MaxLength - 1, ch);
					ret.insert(ret.end(), subpaths.begin(), subpaths.end());
				}
			}
		}
	}

	return ret;
}

struct IdenticalRooms
{
	CMapRoom *GroupRoom;
	set<RoomID> OldRooms;
};

static RoomID get_free_index(unsigned long VNum, unsigned long &index)
{
	while (findRoom(RoomID(VNum, index)))
		index++;
	return RoomID(VNum, index);
}

int CMapRoom::trackMaxLength(const vector< pair<wstring, unsigned long> > &Path) const
{
	if (Path.size() == 0)
		return 0;

	pair<wstring, unsigned long> head = Path[0];

	if (head.second != m_ID.VNum)
		return 0;

	vector< pair<wstring, unsigned long> > tail;
	tail.insert(tail.begin(), Path.begin() + 1, Path.end());

	set<CMapExit>::const_iterator it_exit;
	int maxlen = 0;
	for (it_exit = m_Entrances.begin(); it_exit != m_Entrances.end(); it_exit++) {
		if (it_exit->command() == head.first) {
			CMapRoom *room = findRoom(it_exit->fromID());
			if (room)
				maxlen = max(maxlen, room->trackMaxLength(tail));
		}
	}

	return maxlen + 1;
}

bool CMapRoom::merge(CMapRoom *pOther, RoomID &ChangedRoomID)
{
	unsigned long index = 1;

	map<RoomID, RoomID> old_to_new;
	map<RoomID, set<RoomID> > new_to_old;

	set<RoomID>::iterator it_room;

	IdenticalRooms root;

	root.GroupRoom = new CMapRoom(get_free_index(m_ID.VNum, index), m_Name, m_Descr, m_Area);
	root.OldRooms.insert(m_ID);
	root.OldRooms.insert(pOther->ID());
	
	vector<IdenticalRooms> q;

	q.push_back(root);
	new_to_old[root.GroupRoom->ID()] = root.OldRooms;
	for (it_room = root.OldRooms.begin(); it_room != root.OldRooms.end(); it_room++)
		old_to_new[*it_room] = root.GroupRoom->ID();

	bool ret = true;
	while (q.size() > 0 && ret) {
		IdenticalRooms r = q[0];
		q.erase(q.begin());

		map<wstring, set<RoomID> > group_exits;

		for (it_room = r.OldRooms.begin(); it_room != r.OldRooms.end() && ret; it_room++) {
			CMapRoom *room = findRoom(*it_room);
			if (!room) {
				//???
				continue;
			}
		
			set<CMapExit> exits = room->exits();
			for (set<CMapExit>::const_iterator exit = exits.begin(); exit != exits.end(); exit++) {
				wstring dir = exit->command();
				RoomID toid = exit->toID();
				unsigned long groupped_to = 0;

				if (old_to_new.find(toid) != old_to_new.end())
					toid = old_to_new[toid];
				
				if (group_exits.find(dir) == group_exits.end()) {
					group_exits[dir] = set<RoomID>();
					groupped_to = toid.VNum;
				} else {
					if (toid == r.GroupRoom->ID()) {
						//ignore reflexive non-unique exits
						continue;
					}
					if (group_exits[dir].size() == 1 &&
						*group_exits[dir].begin() == r.GroupRoom->ID()) {
						group_exits[dir].clear();
						groupped_to = toid.VNum;
					} else {
						groupped_to = group_exits[dir].begin()->VNum;
					}
				}

				if (groupped_to != toid.VNum) {
					// merging is impossible
					//ret = false;
					//break;
					continue;
				}

				group_exits[dir].insert(toid);
			}
		}

		if (!ret)
			break;

		// simple union of entrances
		for (it_room = r.OldRooms.begin(); it_room != r.OldRooms.end() && ret; it_room++) {
			CMapRoom *room = findRoom(*it_room);
			if (!room) {
				//???
				continue;
			}
		
			set<CMapExit> entrances = room->entrances();
			for (set<CMapExit>::const_iterator it_exit = entrances.begin(); it_exit != entrances.end(); it_exit++) {
				wstring dir = it_exit->command();
				RoomID fromid = it_exit->fromID();
				if (old_to_new.find(fromid) != old_to_new.end())
					fromid = old_to_new[fromid];
				if (findRoom(fromid))
					r.GroupRoom->addEntrance(fromid, dir);
			}
		}

		for (map<wstring, set<RoomID> >::const_iterator it_exits = group_exits.begin(); it_exits != group_exits.end(); it_exits++) {
			wstring dir = it_exits->first;
			set<RoomID> toids = it_exits->second;

			if (toids.size() == 1) {
				//just add exit
				r.GroupRoom->addExit(*(toids.begin()), dir);
				continue;
			}

			IdenticalRooms new_id;
			set<RoomID>::iterator to_id;

			CMapRoom *info = NULL;
			for (to_id = toids.begin(); to_id != toids.end(); to_id++) {
				if ((info = findRoom(*to_id)) != NULL)
					break;
			}
			if (!info) {
				//???
				continue;
			}

			new_id.GroupRoom = new CMapRoom(get_free_index(info->ID().VNum, index), info->name(), info->descr(), info->area());
			for (to_id = toids.begin(); to_id != toids.end(); to_id++) {
				map<RoomID, set<RoomID> >::iterator it_grouped = new_to_old.find(*to_id);
				if (it_grouped == new_to_old.end()) {
					// ordinary ungroupped room -- just add
					new_id.OldRooms.insert(*to_id);
				} else {
					// already grouped rooms -- add all of them
					for (it_room = it_grouped->second.begin(); it_room != it_grouped->second.end(); it_room++) {
						new_id.OldRooms.insert(*it_room);
						old_to_new[*it_room] = new_id.GroupRoom->ID();
					}

					new_to_old.erase(it_grouped);
					
					CMapRoom *old_group = findRoom(*to_id);
					if (!old_group) {
						//???
						continue;
					}
					old_group->onRoomIDChanged(old_group->ID(), new_id.GroupRoom->ID(), true);
					delete old_group;

					for (vector<IdenticalRooms>::iterator it_queue = q.begin(); it_queue != q.end(); it_queue++)
						if (it_queue->GroupRoom == old_group) {
							q.erase(it_queue);
							break;
						}
				}
			}

			q.push_back(new_id);
			new_to_old[new_id.GroupRoom->ID()] = new_id.OldRooms;
			for (it_room = new_id.OldRooms.begin(); it_room != new_id.OldRooms.end(); it_room++)
				old_to_new[*it_room] = new_id.GroupRoom->ID();

			r.GroupRoom->addExit(new_id.GroupRoom->ID(), dir);
		}
	}

	map<RoomID, set<RoomID> >::iterator it_group;
	if (!ret) {
		for (it_group = new_to_old.begin(); it_group != new_to_old.end(); it_group++) {
			CMapRoom *group = findRoom(it_group->first);
			if (!group) {
				// ???
				continue;
			}
			delete group;
		}
		new_to_old.clear();
	} else if (mesvar[MSG_MAPPER]) {
		int cnt = 0;
		for (it_group = new_to_old.begin(); it_group != new_to_old.end(); it_group++)
			cnt += it_group->second.size();
		
		wchar_t temp[256];
		swprintf(temp, rs::rs(1306), cnt, new_to_old.size());
		tintin_puts2(temp);
	}
	
	for (it_group = new_to_old.begin(); it_group != new_to_old.end(); it_group++) {
		RoomID grpid = it_group->first;

		CMapRoom *group = findRoom(grpid);
		if (!group) {
			//???
			continue;
		}
		
		for (it_room = it_group->second.begin(); it_room != it_group->second.end(); it_room++) {
			RoomID oldid = *it_room;

			set<CMapExit> exits = group->exits(),
				          entrances = group->entrances();
			set<CMapExit>::iterator exit;

			group->onRoomIDChanged(oldid, grpid, true);
			
			CMapRoom *room_old = findRoom(oldid);
			if (room_old && room_old != this) {
				delete room_old;
			}
		}
	}

	if (ret) {
		m_Duplicate = true;
		removeFromList();

		for (it_group = new_to_old.begin(); it_group != new_to_old.end(); it_group++) {
			RoomID grpid = it_group->first;

			CMapRoom *room = findRoom(grpid);
			if (!room) {
				//???
				continue;
			}

			room->autoID(true);
			for (it_room = it_group->second.begin(); it_room != it_group->second.end(); it_room++)
				old_to_new[*it_room] = room->ID();
		}

		//handled in autoID()
		//m_ID = root.GroupRoom->ID();
		//m_Duplicate = true;

		while (old_to_new.find(ChangedRoomID) != old_to_new.end()) {
			if (ChangedRoomID == old_to_new[ChangedRoomID])
				break;
			ChangedRoomID = old_to_new[ChangedRoomID];
		}
	}

	return ret;
}

RoomID CMapRoom::autoID(bool Set) 
{
	RoomID ret(0, 0);
	set<CMapExit>::iterator exit;

	ret.VNum = hash(m_Area + m_Name + m_Descr);
	
	// Calc index based on paths with up-to N lengths and comparing them via vnums only
	vector<unsigned long> paths;
	paths_seen.clear();
	paths = allPaths(Automap_MaxIdenticalExits, Automap_MaxDifferentExits);
	ret.Index = hash(paths);
	
	if (Set)
		m_AutoID = true;
	if (Set && !(m_ID == ret)) {
		CMapRoom *dup = findRoom(ret);

		if (dup) {
			m_Duplicate = true;

			RoomID newid = m_ID;
			if (!merge(dup, newid)) {
				wchar_t temp[BUFFER_SIZE];
				wsprintf(temp, rs::rs(1307), m_ID.toString().c_str(), ret.toString().c_str());
				tintin_puts2(temp);
				m_ID = dup->ID();
			}

			if (!(newid == m_ID)) {
				removeFromList();
				m_ID = newid;
			}
		} else {
			onRoomIDChanged(m_ID, ret, true);
			
			removeFromList();
			m_ID = ret;
			addToList();
		}
	}
	return ret;
}

void CMapRoom::addEntrance(const RoomID &From, const wstring &Direction)
{
	m_Entrances.insert(CMapExit(From, m_ID, Direction));
}

void CMapRoom::delEntrance(const RoomID &From, const wstring &Direction)
{
	set<CMapExit>::iterator it_exit = m_Entrances.find(CMapExit(From, m_ID, Direction));
	if (it_exit != m_Entrances.end())
		m_Entrances.erase(it_exit);
}

bool CMapRoom::addExit(const RoomID &To, const wstring &Direction)
{
	bool changed = false;
	bool recalc_autoid = false;
	set<CMapExit>::iterator it_exit;
	CMapRoom *room_to;

	for (it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (it_exit->command() == Direction) {
			if (!(To == it_exit->toID())) {
				recalc_autoid = true;

				room_to = findRoom(it_exit->toID());
				if (room_to)
					room_to->delEntrance(m_ID, Direction);
				else
					del_delayed_entrance(m_ID, To, Direction);
				room_to = findRoom(To);
				if (room_to)
					room_to->addEntrance(m_ID, Direction);
				else
					add_delayed_entrance(m_ID, To, Direction);
			}
			it_exit->update(m_ID, To);
			changed = true;
			break;
		}
	}
	if (!changed) {
		m_Exits.insert(CMapExit(m_ID, To, Direction));
		room_to = findRoom(To);
		if (room_to)
			room_to->addEntrance(m_ID, Direction);
		else
			add_delayed_entrance(m_ID, To, Direction);
		recalc_autoid = true;
	}
	if (m_AutoID && recalc_autoid) {
		//autoID(true);
		return true;
	}
	return false;
}

bool CMapRoom::delExit(const wstring &Direction)
{
	for (set<CMapExit>::iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (it_exit->command() == Direction) {
			CMapRoom *room_to = findRoom(it_exit->toID());
			if (room_to)
				room_to->delEntrance(m_ID, Direction);
			m_Exits.erase(it_exit);
			return true;
		}
	}
	return false;
}

bool CMapRoom::addPassCommand(const wstring &Direction, const wstring &PassCommand)
{
	for (set<CMapExit>::iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (it_exit->command() == Direction) {
			it_exit->setPassCommand(PassCommand);
			return true;
		}
	}
	return false;
}

void CMapRoom::setComment(const wstring &Comment)
{
	if (m_Comment == Comment)
		return;
	m_Comment = Comment;
}

void CMapRoom::addFlag(const wstring &Flag)
{
	m_Flags.insert(Flag);
}

void CMapRoom::delFlag(const wstring &Flag)
{
	if (m_Flags.find(Flag) != m_Flags.end())
		m_Flags.erase(Flag);
}

void CMapRoom::setAvoidance(int Avoidance)
{
	m_Avoidance = Avoidance;
}

set<wstring> CMapRoom::getFlags() const
{
	return m_Flags;
}

wstring CMapRoom::getAddCommand() const
{
	wstring ret = L"";

	ret += cCommandChar;
	ret += L"mapper add room ";
	
	ret += m_ID.toString() + L' ';
		
	ret += L'{' + m_Name + L'}' + L' ';
	ret += L'{' + m_Descr + L'}' + L' ';
	ret += L'{' + m_Area + L'}' + L' ';
	ret += L'{' + m_Comment + L'}' + L' ';

	wstring flags = L"";
	set<wstring>::const_iterator fit;
	for (fit = m_Flags.begin(); fit != m_Flags.end(); fit++) {
		if (flags.length() > 0)
			flags += L',';
		flags += *fit;
	}
	ret += L'{' + flags + L'}';

	if (m_Avoidance != 1) {
		ret += L'\n';
		ret += cCommandChar;
		ret += L"mapper set avoidance ";
		ret += m_ID.toString() + L' ';

		wchar_t temp[16];
		swprintf(temp, L"%u", m_Avoidance);
		ret += temp;
	}

	for (set<CMapExit>::const_iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		ret += L'\n';
		ret += it_exit->getAddCommand();
		if (it_exit->passCommand().length() > 0) {
			ret += L'\n';
			ret += cCommandChar;
			ret += L"mapper set pass ";
			ret += m_ID.toString() + L' ';
			ret += L'{' + it_exit->command() + L'}' + L' ';
			ret += L'{' + it_exit->passCommand() + L'}';
		}
	}

	return ret;
}

set<CMapExit> CMapRoom::entrances() const
{
	return m_Entrances;
}

set<CMapExit> CMapRoom::exits() const
{
	return m_Exits;
}

CMapExit CMapRoom::getExit(const wstring &Command) const
{
	for (set<CMapExit>::const_iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (it_exit->command() == Command) {
			return *it_exit;
		}
	}
	return CMapExit(m_ID, RoomID(0, 0), Command);
}

CMapExit CMapRoom::getExit(unsigned long ToVNum) const
{
	for (set<CMapExit>::const_iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		if (it_exit->toID().VNum == ToVNum) {
			return *it_exit;
		}
	}
	return CMapExit(m_ID, RoomID(ToVNum, 0), L"");
}

static void get_style_size(RenderStyle style, int &width, int &height)
{
	switch (style) {
	case RenderTiny:
		width = 1;
		height = 1;
		break;
	default:
	case RenderNormal:
		width = 5;
		height = 3;
		break;
	case RenderFull:
		width = 7;
		height = 5;
		break;
	}
}

vector<wstring> CMapRoom::render(RenderStyle style, bool root, bool use_colors) const
{
	vector<wstring> ret;
	int is_n, is_s, is_w, is_e, is_u, is_d, is_nw, is_ne, is_sw, is_se, is_unk;
	is_n = is_s = is_w = is_e = is_u = is_d = is_nw = is_ne = is_sw = is_se = is_unk = 0;

	vector<wstring> signs;
	for (set<CMapExit>::const_iterator it_exit = m_Exits.begin(); it_exit != m_Exits.end(); it_exit++) {
		MapDirection dir = it_exit->direction();

		int door = (it_exit->passCommand().length() > 0) ? 1 : 0;

		if (dir.dX == 0 && dir.dY == 0 && dir.dZ == 0 && dir.Signature.length() == 0)
			is_unk = 1 + door;
		if (dir.dX == 0 && dir.dY  > 0)
			is_n   = 1 + door;
		if (dir.dX == 0 && dir.dY  < 0)
			is_s   = 1 + door;
		if (dir.dX  < 0 && dir.dY == 0)
			is_w   = 1 + door;
		if (dir.dX  > 0 && dir.dY == 0)
			is_e   = 1 + door;
		if (dir.dZ  > 0)
			is_u   = 1 + door;
		if (dir.dZ  < 0)
			is_d   = 1 + door;
		if (dir.dX  < 0 && dir.dY  > 0)
			is_nw  = 1 + door;
		if (dir.dX  > 0 && dir.dY  > 0)
			is_ne  = 1 + door;
		if (dir.dX  < 0 && dir.dY  < 0)
			is_sw  = 1 + door;
		if (dir.dX  > 0 && dir.dY  < 0)
			is_se  = 1 + door;

		if (dir.Signature.length() > 0)
			signs.push_back(dir.Signature);
	}

	for (set<wstring>::const_iterator it_flag = m_Flags.begin(); it_flag != m_Flags.end(); it_flag++) {
		if (RoomFlags.find(*it_flag) != RoomFlags.end())
			if (RoomFlags[*it_flag].Signature.length() > 0) {
				wchar_t sign[BUFFER_SIZE];

				if (RoomFlags[*it_flag].Color.length() > 0 && use_colors)
					add_codes(RoomFlags[*it_flag].Signature.c_str(), sign, RoomFlags[*it_flag].Color.c_str());
				else
					wcscpy(sign,RoomFlags[*it_flag].Signature.c_str()); 

				bool in_esc = false;
				for (int i = 0; sign[i]; i++) {
					if (sign[i] == L'\x1B' && sign[i + 1] == L'[')
						in_esc = true;
					else if (in_esc && sign[i] == L'm')
						in_esc = false;
					else if (!in_esc) {
						sign[i + 1] = L'\0';
						break;
					}
				}
				
				signs.push_back(sign);
			}
	}

	int w, h;
	get_style_size(style, w, h);
	for (int i = 0; i < h; i++)
		ret.push_back(L"");

	wstring color = L"";
	if (use_colors)
		color = L"\x1B[1;37m";

	switch (style) {
	case RenderTiny:
		ret[0] += color;

		if (root)
			ret[0] += L"#";
		else if (is_unk)
			ret[0] += L"?";
		else if (is_n && is_s && is_e && is_w)
			ret[0] += L"+";
		else if (is_n && is_s && !is_w && !is_e)
			ret[0] += L"|";
		else if (!is_n && !is_s && is_w && is_e)
			ret[0] += L"-";
		else if (is_u && !is_d)
			ret[0] += L"^";
		else if (!is_u && is_d)
			ret[0] += L"v";
		else if (signs.size() > 0)
			ret[0] = signs[0];
		else
			ret[0] = L"o";

		break;
	default:
	case RenderNormal:
		ret[1] += color;
		if (is_w)  ret[1] += (is_w  == 2 ? L"+" : L"-");
		else       ret[1] += L" ";
		ret[1] += L"(";
		if (root)  ret[1] += L"#";
		else if (signs.size() > 0) ret[1] += signs[0] + color, signs.erase(signs.begin());
		else       ret[1] += L" ";
		ret[1] += L")";
		if (is_e)  ret[1] += (is_e  == 2 ? L"+" : L"-");
		else       ret[1] += L" ";

		ret[0] += color;
		if (is_nw) ret[0] += (is_nw == 2 ? L"X" : L"\\");
		else       ret[0] += L" ";
		ret[0] += L" ";
		if (is_n)  ret[0] += (is_n  == 2 ? L"+" : L"|");
		else       ret[0] += L" ";
		if (is_u)  ret[0] += L"^";
		else if (signs.size() > 0) ret[0] += signs[0] + color, signs.erase(signs.begin());
		else       ret[0] += L" ";
		if (is_ne) ret[0] += (is_ne  == 2 ? L"X" : L"/");
		else       ret[0] += L" ";

		ret[2] += color;
		if (is_sw) ret[2] += (is_sw  == 2 ? L"X" : L"/");
		else       ret[2] += L" ";
		if (is_d)  ret[2] += L"v";
		else if (signs.size() > 0) ret[2] += signs[0] + color, signs.erase(signs.begin());
		else       ret[2] += L" ";
		if (is_s)  ret[2] += (is_s  == 2 ? L"+" : L"|");
		else       ret[2] += L" ";
		if (is_unk) ret[2] += L"?";
		else if (signs.size() > 0) ret[2] += signs[0] + color, signs.erase(signs.begin());
		else       ret[2] += L" ";
		if (is_se) ret[2] += (is_se == 2 ? L"X" : L"\\");
		else       ret[2] += L" ";
		
		break;
	case RenderFull:
		ret[0] += color;
		ret[0] += (is_nw == 2 ? L"X" : is_nw ? L"\\" : L" ");
		ret[0] += L" ";
		ret[0] += L" ";
		ret[0] += (is_n  == 2 ? L"+" : is_n  ? L"|" : L" ");
		ret[0] += L" ";
		ret[0] += L" ";
		ret[0] += (is_ne == 2 ? L"X" : is_ne ? L"/" : L" ");

		ret[1] += color;
		ret[1] += L" ";
		ret[1] += L" ";
		ret[1] += (is_unk ? L"?" : L" ");
		if (signs.size() > 0) ret[1] += signs[0], signs.erase(signs.begin()); else ret[1] += L" ";
		ret[1] += (is_u   ? L"^" : L" ");
		if (signs.size() > 0) ret[1] += signs[0], signs.erase(signs.begin()); else ret[1] += L" ";
		ret[1] += L" ";

		ret[2] += color;
		ret[2] += (is_w  == 2 ? L"+" : is_w  ? L"-" : L" ");
		if (signs.size() > 0) ret[2] += signs[0], signs.erase(signs.begin()); else ret[2] += L" ";
		ret[2] += L"(";
		ret[2] += (root ? L"#" : L" ");
		ret[2] += L")";
		if (signs.size() > 0) ret[2] += signs[0], signs.erase(signs.begin()); else ret[2] += L" ";
		ret[2] += (is_e  == 2 ? L"+" : is_e  ? L"-" : L" ");

		ret[3] += color;
		ret[3] += L" ";
		if (signs.size() > 0) ret[3] += signs[0], signs.erase(signs.begin()); else ret[3] += L" ";
		ret[3] += (is_u   ? L"v" : L" ");
		if (signs.size() > 0) ret[3] += signs[0], signs.erase(signs.begin()); else ret[3] += L" ";
		if (signs.size() > 0) ret[3] += signs[0], signs.erase(signs.begin()); else ret[3] += L" ";
		if (signs.size() > 0) ret[3] += signs[0], signs.erase(signs.begin()); else ret[3] += L" ";
		ret[3] += L" ";

		ret[4] += color;
		ret[4] += (is_sw == 2 ? L"X" : is_sw ? L"/" : L" ");
		ret[4] += L" ";
		ret[4] += L" ";
		ret[4] += (is_s  == 2 ? L"+" : is_s  ? L"|" : L" ");
		ret[4] += L" ";
		ret[4] += L" ";
		ret[4] += (is_se == 2 ? L"X" : is_se ? L"\\" : L" ");

		break;
	}
	return ret;
}

bool CMapRoom::autoID() const
{
	return m_AutoID;
}

bool CMapRoom::duplicate() const
{
	return m_Duplicate;
}

wstring CMapRoom::name() const
{
	return m_Name;
}

wstring CMapRoom::descr() const
{
	return m_Descr;
}

wstring CMapRoom::area() const
{
	return m_Area;
}

wstring CMapRoom::comment() const
{
	return m_Comment;
}

int CMapRoom::avoidance() const
{
	return m_Avoidance;
}

void CMapRoom::update(const wstring &Name, const wstring &Descr, const wstring &Area, const wstring &Comment, const wstring &Flags)
{
	if (Name.length() > 0)
		m_Name = Name;
	if (Descr.length() > 0)
		m_Descr = Descr;
	if (Area.length() > 0)
		m_Area = Area;
	if (Comment.length() > 0)
		m_Comment = Comment;
	vector<wstring> flags_str = split_str(Flags.c_str(), L',');
	for (int i = 0; i < flags_str.size(); i++)
		m_Flags.insert(flags_str[i]);
}

CMapRoom::CMapRoom(const RoomID &ID, const wstring &Name, const wstring &Descr, 
				   const wstring &Area, const wstring &Comment, const wstring &Flags):
	m_ID(ID), m_Name(Name), m_Descr(Descr), m_Area(Area), m_Comment(Comment), m_Avoidance(1)
{
	if (m_ID.VNum == 0) {
		m_ID = autoID(false);
		m_AutoID = true;
	} else {
		m_AutoID = false;
	}

	m_Duplicate = false;
	if (findRoom(m_ID)) {
		m_Duplicate = true;
	}

	if (!m_Duplicate) {
		vector<wstring> flags_str = split_str(Flags.c_str(), L',');
		for (int i = 0; i < flags_str.size(); i++)
			m_Flags.insert(flags_str[i]);
		addToList();

		map< RoomID, set<CMapExit> >::iterator it_entrances;
		it_entrances = DelayedEntrances.find(m_ID);
		if (it_entrances != DelayedEntrances.end()) {
			m_Entrances = it_entrances->second;
			DelayedEntrances.erase(it_entrances);
		}
	}
}

CMapRoom::~CMapRoom()
{
	if (!m_Duplicate)
		removeFromList();
}

static vector<wstring> render_spacer_room(RenderStyle style)
{
	vector<wstring> ret;
	int w, h;
	get_style_size(style, w, h);
	wstring row;
	row.resize(w, L' ');
	for (int i = 0; i < h; i++)
		ret.push_back(row);
	return ret;
}

static vector<wstring> render_null_room(RenderStyle style, const RoomID &ID, bool use_color)
{
	vector<wstring> ret;
	wstring color = L"";
	if (use_color)
		color = L"\x1B[1;30m";
	switch (style) {
	case RenderTiny:
		ret.push_back(color + L"?");
		break;
	default:
	case RenderNormal:
		ret.push_back(color + L". . .");
		ret.push_back(color + L". ? .");
		ret.push_back(color + L". . .");
		break;
	case RenderFull:
		ret.push_back(color + L".  .  .");
		ret.push_back(color + L"       ");
		ret.push_back(color + L".  ?  .");
		ret.push_back(color + L"       ");
		ret.push_back(color + L".  .  .");
		break;
	}
	return ret;
}

struct PlacedRoom
{
	RoomID ID;
	CMapRoom *pRoom;
	int x, y;

	void clear() { ID = RoomID(0, 0); };
	void setRoom(const RoomID &id) {ID = id; pRoom = findRoom(id); };
	vector<wstring> render(RenderStyle style, const RoomID &ID0, bool use_colors) {
		if (!(ID == RoomID(0, 0))) {
			if (pRoom)
				return pRoom->render(style, ID == ID0, use_colors);
			else
				return render_null_room(style, ID, use_colors);
		}
		return render_spacer_room(style);
	};
	bool empty() { return (ID == RoomID(0,0)); };

	PlacedRoom(): pRoom(NULL), x(0), y(0), ID(RoomID(0, 0)) {};
	
};

enum RenderFlags
{
	RenderPrintHeader	= (1 << 0),
	RenderPrintDescr	= (1 << 1),
	RenderPrintExits	= (1 << 2),
	RenderPrintFlags	= (1 << 3),
	RenderPrintComment	= (1 << 4)
};

static vector<wstring> render_map(const RoomID &ID0, int width, int height, bool use_colors, 
								  bool cropped, RenderStyle style, int Flags = 0)
{
	int style_w, style_h;
	int x, y;
	get_style_size(style, style_w, style_h);

	int x0 = (width - style_w) / 2;
	int y0 = (height - style_h) / 2;

	vector<wstring> headers, footers;

	CMapRoom *root = findRoom(ID0);
	if (root) {
		if (Flags & RenderPrintHeader) {
			wstring header = L"\x1B[1;37m" + root->name();
			if (root->area().length() > 0)
				header += L" [" + root->area() + L"]";
			if (header.length() > 0)
				headers.push_back(header);
		}
		if (Flags & RenderPrintDescr && root->descr().length() > 0) {
			headers.push_back(L"\x1B[0;37m" + root->descr());
		}
		if (Flags & RenderPrintExits) {
			set<CMapExit> exits = root->exits();
			wstring footer = L"";
			for (set<CMapExit>::const_iterator it_exit = exits.begin(); it_exit != exits.end(); it_exit++) {
				if (footer.length() == 0)
					footer += L"\x1B[1;37m[";
				else
					footer += L", ";
				footer += it_exit->command();
				if (it_exit->passCommand().length() > 0)
					footer += L" \x1B[0;37m(" + it_exit->passCommand() + L")\x1B[1;37m";
			}
			if (footer.length() > 0)
				footer += L"]";
			else
				footer = L"\x1B[1;37m[ - ]";
			footers.push_back(footer);
		}
		if (Flags & RenderPrintFlags) {
			set<wstring> flist = root->getFlags();
			wstring footer = L"";
			for (set<wstring>::const_iterator it_flag = flist.begin(); it_flag != flist.end(); it_flag++) {
				if (footer.length() == 0)
					footer += L"\x1B[1;37m";
				else
					footer += L",";
				footer += *it_flag;
			}
			if (footer.length() > 0)
				footers.push_back(footer);
		}
		if (Flags & RenderPrintComment && root->comment().length() > 0) {
			footers.push_back(L"\x1B[0;37m" + root->comment());
		}
	}

	int i;
	for (i = 0; i < headers.size(); i++)
		height -= headers[i].length() / width + 1;
	for (i = 0; i < footers.size(); i++)
		height -= footers[i].length() / width + 1;

	if (height < style_h)
		height = style_h;

	int max_dx = (width / 2) / style_w + 1;
	int max_dy = (height / 2) / style_h + 1;

	int grid_w = (max_dx * 2 + 1);
	int grid_h = (max_dy * 2 + 1);

	PlacedRoom *render_grid = new PlacedRoom[grid_w * grid_h];
	for (y = 0; y < grid_h; y++)
		for (x = 0; x < grid_w; x++) {
			render_grid[y * grid_w + x].clear();
			render_grid[y * grid_w + x].x = x;
			render_grid[y * grid_w + x].y = y;
		}
	map <RoomID, PlacedRoom*> placed;

	map <RoomID, PlacedRoom*>::iterator it_placed;
	vector <RoomID> q;
	set< pair<RoomID, RoomID> > other_exits;

	render_grid[max_dy * grid_w + max_dx].setRoom(ID0);
	placed[ID0] = &render_grid[max_dy * grid_w + max_dx];
	q.push_back(ID0);

	while (q.size() > 0) {
		RoomID r_id = q[0];
		q.erase(q.begin());

		it_placed = placed.find(r_id);
		if (it_placed == placed.end()) 
			continue;

		PlacedRoom *r = it_placed->second;
		if (!r->pRoom) 
			continue;

		set<CMapExit> exits = r->pRoom->exits();
		set<CMapExit> entrances = r->pRoom->entrances();
		set<CMapExit>::const_iterator it_exit;
		for (it_exit = entrances.begin(); it_exit != entrances.end(); it_exit++)
			exits.insert(*it_exit);

		for (it_exit = exits.begin(); it_exit != exits.end(); it_exit++) {
			MapDirection dir = it_exit->direction();
			RoomID next;
			int dx, dy;
				
			if (it_exit->fromID() == r->ID) {
				next = it_exit->toID();
				dx = dir.dX;
				dy = dir.dY;
			} else {
				next = it_exit->fromID();
				dx = -dir.dX;
				dy = -dir.dY;
			}

			if (dx == 0 && dy == 0) {
				other_exits.insert(pair<RoomID, RoomID>(r->ID, next));
				continue;
			}

			int x = r->x + dx;
			int y = r->y + dy;

			if (x < 0 || x >= grid_w || y < 0 || y >= grid_h)
				continue;
			if (!render_grid[y * grid_w + x].empty())
				continue;
			if (placed.find(next) != placed.end())
				continue;

			render_grid[y * grid_w + x].setRoom(next);
			placed[next] = &render_grid[y * grid_w + x];
			q.push_back(next);
		}

		while (q.size() == 0 && other_exits.size() > 0) {
			pair<RoomID, RoomID> exit = *(other_exits.begin());
			other_exits.erase(other_exits.begin());
			if (placed.find(exit.second) != placed.end())
				continue;
			it_placed = placed.find(exit.first);
			if (it_placed != placed.end()) {
				PlacedRoom *r = it_placed->second;
				for (int dx = -1; dx <= +1 && q.size() == 0; dx++)
					for (int dy = -1; dy <= +1 && q.size() == 0; dy++) {
						int xx = r->x + dx,
							yy = r->y + dy;
						if (xx < 0 || xx >= grid_w || yy < 0 || yy >= grid_h)
							continue;
						if (!render_grid[yy * grid_w + xx].empty())
							continue;
						render_grid[yy * grid_w + xx].setRoom(exit.second);
						placed[exit.second] = &render_grid[yy * grid_w + xx];
						q.push_back(exit.second);
					}
			}
		}
	}

	int minx = 0, maxx = grid_w - 1,
		miny = 0, maxy = grid_h - 1;

	if (cropped) {
		minx = grid_w - 1, maxx = 0;
		miny = grid_h - 1, maxy = 0;
		for (y = 0; y < grid_h; y++) {
			bool empty_line = true;
			bool first_room = true;
			for (x = 0; x < grid_w; x++) {
				if (!render_grid[y*grid_w + x].empty()) {
					empty_line = false;
					if (first_room) {
						minx = min(minx, x);
						first_room = false;
					}
					maxx = max(maxx, x);
				}
			}
			if (!empty_line) {
				miny = min(miny, y);
				maxy = max(maxy, y);
			}
		}
	}

	vector<wstring> ret;
	for (y = miny; y <= maxy; y++) {
		bool row_inserted = false;
		for (x = minx; x <= maxx; x++) {
			int ox = x0 + (x - max_dx) * style_w;
			if (ox < 0 || (ox + style_w >= width))
				continue;

			int oy = y0 + (y - max_dy) * style_h;
			if (oy < 0 || (oy + style_h >= height))
				continue;
			
			vector<wstring> room = render_grid[y*grid_w + x].render(style, ID0, use_colors);

			if (!row_inserted) {
				for (int yy = 0; yy < style_h; yy++)
					ret.insert(ret.begin(), L"");
				row_inserted = true;
			}
			if (use_colors)
				for (int yy = 0; yy < style_h; yy++)
					ret[yy] += L"\x1b[0m";
			for (int yy = 0; yy < style_h; yy++)
				ret[yy] += room[yy];
		}
	}

	for (i = headers.size() - 1; i >= 0; i--)
		ret.insert(ret.begin(), headers[i]);
	for (i = 0; i < footers.size(); i++)
		ret.push_back(footers[i]);
	
	delete[] render_grid;

	return ret;
}

//**** helpers for mapper subcommands ***//

class CRoomFilter
{
private:
	enum FilterField
	{
		Filter_ByName,
		Filter_ByArea,
		Filter_ByDescr,
		Filter_ByComment,
		Filter_ByFlag,
		Filter_Unknown
	};

	FilterField m_Field;
	wstring m_Value;

public:
	CRoomFilter(const wchar_t *key, const wchar_t *value) {
		if (is_abrev(key, L"name"))
			m_Field = Filter_ByName;
		else if (is_abrev(key, L"area"))
			m_Field = Filter_ByArea;
		else if (is_abrev(key, L"description"))
			m_Field = Filter_ByDescr;
		else if (is_abrev(key, L"comment"))
			m_Field = Filter_ByComment;
		else if (is_abrev(key, L"flag"))
			m_Field = Filter_ByFlag;
		else
			m_Field = Filter_Unknown;

		m_Value = value;
	}

	bool valid() const { return m_Field != Filter_Unknown; }
	bool check(const CMapRoom *room) const {
		switch (m_Field) {
		case Filter_ByName:
			return room->name().find(m_Value) != wstring::npos;
		case Filter_ByArea:
			return room->area().find(m_Value) != wstring::npos;
		case Filter_ByDescr:
			return room->descr().find(m_Value) != wstring::npos;
		case Filter_ByComment:
			return room->comment().find(m_Value) != wstring::npos;
		case Filter_ByFlag:
			{
				set<wstring> flags = room->getFlags();
				return flags.find(m_Value) != flags.end();
			}
		}
		return false;
	}
};

static void set_variable(const wchar_t *varname, const wchar_t *value)
{
	if (varname[0]) {
		VAR_INDEX ind = VarList.find(varname);
		VAR* pvar;
		if (ind != VarList.end()) {
			pvar = ind->second;
			pvar->m_strVal = value;
		} else {
			pvar = new VAR(value);
			VarList[varname] = pvar;
		}
	}
}
static wstring arg_with_vars(wchar_t **arg)
{
	wchar_t temp[BUFFER_SIZE], val[BUFFER_SIZE];
	*arg = get_arg_in_braces(*arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, val, sizeof(val) / sizeof(wchar_t) - 1);
	return wstring(val);
}
static RoomID room_id_arg(wchar_t **arg, const wchar_t *subcommand)
{
	wchar_t temp[BUFFER_SIZE];
	wstring val = arg_with_vars(arg);
	RoomID id;
	if (!RoomID::fromString(val.c_str(), id)) {
		swprintf(temp, rs::rs(1309), subcommand);
		tintin_puts2(temp);
		return RoomID(0, 0);
	}
	return id;
}
static CMapRoom *room_arg(wchar_t **arg, const wchar_t *subcommand)
{
	wchar_t temp[BUFFER_SIZE];
	CMapRoom *room = findRoom(room_id_arg(arg, subcommand));
	if (!room) {
		swprintf(temp, rs::rs(1310), subcommand);
		tintin_puts2(temp);
	}
	return room;
}
static wstring direction_arg(wchar_t **arg)
{
	wstring val = arg_with_vars(arg);
	if (SynonymToDirection.find(val) != SynonymToDirection.end())
		return SynonymToDirection[val];
	return val;
}
static void add_autoexit(CMapRoom *room_from, CMapRoom **room_to, const wstring &command, RoomID &fromID, RoomID &toID)
{
	if (room_from->addExit(toID, command)) { //ID could be changed
		set<RoomID> seen;
		vector<RoomID> q;
		q.push_back(fromID);
		seen.insert(fromID);

		while (q.size() > 0) {
			RoomID r = q[0];
			q.erase(q.begin());

			CMapRoom *room = findRoom(r);
			if (!room) {
				//????
				continue;
			}
			if (room->autoID()) {
				// merge "manually" (not inside autoID())
				// because we need to know how fromID changed during merging
				RoomID newid = room->autoID(false);
				if (!(newid == r)) {
					CMapRoom *dup = findRoom(newid);
					if (dup) {
						if (room->merge(dup, fromID)) {
							delete room;
							room_from = findRoom(fromID);
							break;
						}
					}
					room->autoID(true);
					if (room == room_from)
						fromID = room->ID();

					if (room) {
						set<CMapExit> entrances = room->entrances();
					for (set<CMapExit>::const_iterator it_exit = entrances.begin(); it_exit != entrances.end(); it_exit++) {
						if (seen.find(it_exit->fromID()) == seen.end()) {
								q.push_back(it_exit->fromID());
								seen.insert(it_exit->fromID());
							}
						}
					}
				}
			}
		}
	}
	fromID = room_from->ID();
	toID = room_from->getExit(command).toID();
	*room_to = findRoom(toID);
}

//**** mapper subcommands ***//

static void mapper_clear()
{
	while (Rooms.size() > 0) {
		map<unsigned long, map<unsigned long, CMapRoom*> >::iterator it_room = Rooms.begin();
		int i = it_room->second.size();
		while (i > 0) {
			delete it_room->second.begin()->second;
			--i;
		}
	}
	DelayedEntrances.clear();
	Directions.clear();
	SynonymToDirection.clear();
	DirectionToSynonyms.clear();
	RoomFlags.clear();
	TrackHistory.clear();
}

static void mapper_write(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE];
	wstring fname = arg_with_vars(&arg);
	wstring contents = L"";
	
	map<wstring, MapDirection>::const_iterator it_dir;
	for (it_dir = Directions.begin(); it_dir != Directions.end(); it_dir++) {
		MapDirection dir = it_dir->second;
		wchar_t tmp[64];
		map<wstring, SetOfSynonyms>::const_iterator it_syns;
		set<wstring>::const_iterator it_syn;
		wstring synonyms;

		contents += cCommandChar;
		contents += L"mapper add direction {";
		contents += dir.Command;

		it_syns = DirectionToSynonyms.find(dir.Command);
		if (it_syns != DirectionToSynonyms.end()) {
			for (it_syn = it_syns->second.synonyms.begin(); it_syn != it_syns->second.synonyms.end(); it_syn++)
				if (*it_syn != dir.Command) {
					contents += L",";
					contents += *it_syn;
				}
		}

		contents += L"} ";
		swprintf(tmp, L"%d %d %d", dir.dX, dir.dY, dir.dZ);
		contents += tmp;
		contents += L" {";
		contents += dir.Reversed;

		it_syns = DirectionToSynonyms.find(dir.Reversed);
		if (it_syns != DirectionToSynonyms.end()) {
			for (it_syn = it_syns->second.synonyms.begin(); it_syn != it_syns->second.synonyms.end(); it_syn++)
				if (*it_syn != dir.Reversed) {
					contents += L",";
					contents += *it_syn;
				}
		}

		contents += L"}";
		contents += L" {";
		contents += dir.Signature;
		contents += L"}\n";
	}

	map<wstring, MapRoomFlag>::const_iterator it_flag;
	for (it_flag = RoomFlags.begin(); it_flag != RoomFlags.end(); it_flag++) {
		MapRoomFlag flag = it_flag->second;
		
		contents += cCommandChar;
		contents += L"mapper add flag {";
		contents += flag.Name;
		contents += L"}";
		contents += L" {";
		contents += flag.Signature;
		contents += L"}";
		contents += L" {";
		contents += flag.Color;
		contents += L"}\n";
	}

	map<unsigned long, map<unsigned long, CMapRoom*> >::const_iterator it_vnum;
	map<unsigned long, CMapRoom*>::const_iterator it_index;
	int rooms_count = 0;
	for (it_vnum = Rooms.begin(); it_vnum != Rooms.end(); it_vnum++) {
		for (it_index = it_vnum->second.begin(); it_index != it_vnum->second.end(); it_index++) {
			contents += it_index->second->getAddCommand() + L'\n';
			rooms_count++;
		}
	}

	wchar_t fn[MAX_PATH+2];
    MakeAbsolutePath(fn, fname.c_str(), szBASE_DIR);
	int bytes = write_file_contents(fn, contents.c_str(), contents.length());
	
	swprintf(temp, rs::rs(1308), fname.c_str(), bytes, rooms_count);

	tintin_puts2(temp);
}

static void mapper_track(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		
	if (is_abrev(temp, L"add")) {
		wstring command = direction_arg(&arg);
		RoomID id = room_id_arg(&arg, L"track add");
		if (!(id == RoomID(0, 0))) {
			TrackHistory.insert(TrackHistory.begin(), pair< wstring, unsigned long >(command, id.VNum));
			int maxlen = max(Automap_MaxDifferentExits, Automap_MaxIdenticalExits) * 2;
			while (TrackHistory.size() > maxlen)
				TrackHistory.pop_back();
		}
	} else if (is_abrev(temp, L"clear")) {
		TrackHistory.clear();
	} else if (is_abrev(temp, L"position")) {
		// try to find room according to TrackHistory
		if (TrackHistory.size() == 0) {
			tintin_puts2(rs::rs(1311));
			return;
		}

		unsigned long vnum = TrackHistory[0].second;
		map <unsigned long, CMapRoom*> candidates = Rooms[vnum];
		map <unsigned long, CMapRoom*>::const_iterator it_cand;

		RoomID bestid;
		int bestlen = 0;
		for (it_cand = candidates.begin(); it_cand != candidates.end(); it_cand++) {
			int len = it_cand->second->trackMaxLength(TrackHistory);
			if (len > bestlen) {
				bestlen = len;
				bestid = it_cand->second->ID();
			}
		}

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		set_variable(temp, bestid.toString().c_str());
	} else {
		tintin_puts2(rs::rs(1312));
	}
}

static void mapper_autoid(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE];

	wstring name = arg_with_vars(&arg);
	wstring descr = arg_with_vars(&arg);
	wstring area = arg_with_vars(&arg);

	unsigned long vnum = hash(area + name + descr);
	RoomID id(vnum, 0);
	CMapRoom *room = findRoom(id);
	if (room)
		id = room->ID();

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	set_variable(temp, id.toString().c_str());
}

static void mapper_del(wchar_t *arg)
{
	wchar_t obj[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, obj, sizeof(obj) / sizeof(wchar_t) - 1);

	if (is_abrev(obj, L"room")) {
		CMapRoom *room = room_arg(&arg, L"del room");
		if (room) {
			delete room;
			if (mesvar[MSG_MAPPER])
				tintin_puts2(rs::rs(1316));
		}
	} else if (is_abrev(obj, L"exit")) {
		CMapRoom *room = room_arg(&arg, L"del exit");
		if (room) {
			wstring command = direction_arg(&arg);
			if (room->delExit(command) && room->autoID())
				room->autoID(true);
			if (mesvar[MSG_MAPPER])
				tintin_puts2(rs::rs(1317));
		}
	} else {
		tintin_puts2(rs::rs(1318));
	}
}

static void mapper_merge(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE];

	CMapRoom *room1, *room2;
	
	if ((room1 = room_arg(&arg, L"merge")) &&
		(room2 = room_arg(&arg, L"merge")) ) {
		if (room1->ID() == room2->ID()) {
			tintin_puts2(rs::rs(1322));
			return;
		}
		RoomID newid = room1->ID();
		room1->merge(room2, newid);

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		set_variable(temp, newid.toString().c_str());
	}
}

static void mapper_add(wchar_t *arg)
{
	wchar_t obj[BUFFER_SIZE], vnum[BUFFER_SIZE], strng[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, obj, sizeof(obj) / sizeof(wchar_t) - 1);

	if (is_abrev(obj, L"room")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, vnum, sizeof(vnum) / sizeof(wchar_t) - 1);

		RoomID id;
		if (!RoomID::fromString(vnum, id)) {
			if (is_abrev(vnum, L"auto")) {
				id = RoomID(0, 0);
			} else {
				tintin_puts2(rs::rs(1313));
				return;
			}
		}
		
		wstring name = arg_with_vars(&arg);
		wstring descr = arg_with_vars(&arg);
		wstring area = arg_with_vars(&arg);
		wstring comment = arg_with_vars(&arg);
		wstring flags = arg_with_vars(&arg);

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);

		CMapRoom *room = NULL;
			
		if (!(id == RoomID(0, 0)))
			room = findRoom(id);
		if (room && (room->ID() == id)) {
			room->update(name, descr, area, comment, flags);
			if (room->autoID())
				room->autoID(true);
		} else {
			room = new CMapRoom(id, name, descr, area, comment, flags);
		}

		set_variable(temp, room->ID().toString().c_str());

		if (room->duplicate()) {
			swprintf(temp, rs::rs(1314), room->ID().toString().c_str());
			delete room;
		} else {
			swprintf(temp, rs::rs(1315), room->ID().toString().c_str());
			//swprintf(temp, L"added new room %ls, total rooms: %d", room->ID().toString().c_str(), Rooms.size());
		}
			
		if (mesvar[MSG_MAPPER])
			tintin_puts2(temp);
	} else if (is_abrev(obj, L"exit")) {
		CMapRoom *room_from = room_arg(&arg, L"add exit");
		if (!room_from)
			return;
		wstring command = direction_arg(&arg);
		RoomID toID = room_id_arg(&arg, L"add exit");

		RoomID fromID = room_from->ID();
		CMapRoom *room_to = findRoom(toID);

		bool force_autoid_to = false;
		if (fromID == toID && room_from->autoID()) {
			// special case: automapping without VNum,
			// with asymmetric exits it is impossible to create connection like A1 -> A2 where A is same name+descr+area
			// i.e. even simpliest passages can't be created, so:
			// 1) first create new room_to without auto-id;
			// 2) then connect from -> to
			// 3) add exit to from-room
			// 4) give to room_to auto-id
			unsigned long index = 1;
			toID = get_free_index(fromID.VNum, index);
			room_to = new CMapRoom(toID, room_from->name(), room_from->descr(), room_from->area());
			force_autoid_to = true;
		}

		add_autoexit(room_from, &room_to, command, fromID, toID);

		if (room_to && force_autoid_to)
			toID = room_to->autoID(true);

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		if (is_abrev(temp, L"bidirectional") && Directions.find(command) != Directions.end() && room_to) {
			wstring revcmd = Directions[command].Reversed;
			if (revcmd.length() > 0)
				add_autoexit(room_to, &room_from, revcmd, toID, fromID);
			arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		}
		set_variable(temp, fromID.toString().c_str());
		
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		set_variable(temp, toID.toString().c_str());
	} else if (is_abrev(obj, L"direction")) {
		arg = get_arg_in_braces(arg, strng, STOP_SPACES, sizeof(strng) / sizeof(wchar_t) - 1);

		MapDirection dir;

		int i;
		vector<wstring> synonyms = split_str(strng, L',');
		dir.Command = synonyms[0];
		DirectionToSynonyms[dir.Command].synonyms = set<wstring>();
		for (i = 0; i < synonyms.size(); i++) {
			SynonymToDirection[synonyms[i]] = dir.Command;
			DirectionToSynonyms[dir.Command].synonyms.insert(synonyms[i]);
		}

		dir.Reversed = L"";
		dir.Signature = L"";
		dir.dX = dir.dY = dir.dZ = 0;

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		if (is_all_digits(temp, true))
			swscanf(temp, L"%d", &dir.dX);
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		if (is_all_digits(temp, true))
			swscanf(temp, L"%d", &dir.dY);
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		if (is_all_digits(temp, true))
			swscanf(temp, L"%d", &dir.dZ);

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		synonyms = split_str(temp, L',');
		dir.Reversed = synonyms[0];
		DirectionToSynonyms[dir.Reversed].synonyms = set<wstring>();
		for (i = 0; i < synonyms.size(); i++) {
			SynonymToDirection[synonyms[i]] = dir.Reversed;
			DirectionToSynonyms[dir.Reversed].synonyms.insert(synonyms[i]);
		}

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		dir.Signature = temp;

		if (dir.Command.length() > 0) {
			Directions[dir.Command] = dir;

			if (dir.Reversed.length() > 0) {
				MapDirection revdir;

				revdir.Command = dir.Reversed;
				revdir.Reversed = dir.Command;
				revdir.dX = -dir.dX;
				revdir.dY = -dir.dY;
				revdir.dZ = -dir.dZ;
				revdir.Signature = dir.Signature;
				Directions[revdir.Command] = revdir;
			}
		} else {
			tintin_puts2(rs::rs(1319));
		}
	} else if (is_abrev(obj, L"flag")) {
		arg = get_arg_in_braces(arg, strng, STOP_SPACES, sizeof(strng) / sizeof(wchar_t) - 1);

		MapRoomFlag flag;
		flag.Name = strng;

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		flag.Signature = temp;

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		flag.Color = temp;

		if (wcslen(strng) > 0) {
			RoomFlags[strng] = flag;
		} else {
			tintin_puts2(rs::rs(1320));
		}
	} else {
		tintin_puts2(rs::rs(1321));
	}
}

static void mapper_set(wchar_t *arg)
{
	wchar_t obj[BUFFER_SIZE], strng[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, obj, sizeof(obj) / sizeof(wchar_t) - 1);

	if (is_abrev(obj, L"flag")) {
		CMapRoom *room = room_arg(&arg, L"set flag");
		if (room) {
			wstring flag = arg_with_vars(&arg);
			room->addFlag(flag.c_str());
			if (mesvar[MSG_MAPPER]) {
				swprintf(temp, rs::rs(1324), flag.c_str(), room->ID().toString().c_str());
				tintin_puts2(temp);
			}
		}
	} else if (is_abrev(obj, L"comment")) {
		CMapRoom *room = room_arg(&arg, L"set comment");
		if (room) {
			wstring comment = arg_with_vars(&arg);
			room->setComment(comment);
			if (mesvar[MSG_MAPPER]) {
				swprintf(temp, rs::rs(1327), room->ID().toString().c_str());
				tintin_puts2(temp);
			}
		}
	} else if (is_abrev(obj, L"avoidance")) {
		CMapRoom *room = room_arg(&arg, L"set avoidance");
		if (room) {
			arg = get_arg_in_braces(arg, temp, WITH_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
			prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);
			if (!is_all_digits(strng)) {
				tintin_puts2(rs::rs(1323));
				return;
			}
			room->setAvoidance(_wtoi(strng));
			if (mesvar[MSG_MAPPER]) {
				swprintf(temp, rs::rs(1325), room->ID().toString().c_str());
				tintin_puts2(temp);
			}
		}
	} else if (is_abrev(obj, L"pass")) {
		CMapRoom *room = room_arg(&arg, L"set pass");
		if (room) {
			wstring direction = direction_arg(&arg);
			wstring pass = arg_with_vars(&arg);
			if (!room->addPassCommand(direction, pass)) {
				swprintf(temp, rs::rs(1330), direction.c_str(), room->ID().toString().c_str());
			} else {
				swprintf(temp, rs::rs(1331), direction.c_str(), room->ID().toString().c_str(), direction.c_str());
			}
			if (mesvar[MSG_MAPPER])
				tintin_puts2(temp);
		}
	} else if (is_abrev(obj, L"maxdifflen")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);

		if (!is_all_digits(strng)) {
			swprintf(temp, rs::rs(1332), L"maxdifflen", 1, 100);
		} else {
			unsigned long tmp;
			swscanf(strng, L"%u", &tmp);
			if (tmp < 1 || tmp > 100) {
				swprintf(temp, rs::rs(1332), L"maxdifflen", 1, 100);
			} else {
				Automap_MaxDifferentExits = tmp;
				swprintf(temp, rs::rs(1333), L"maxdifflen", Automap_MaxDifferentExits);
			}
		}

		if (mesvar[MSG_MAPPER])
			tintin_puts2(temp);
	} else if (is_abrev(obj, L"maxidentlen")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);

		if (!is_all_digits(strng)) {
			swprintf(temp, rs::rs(1332), L"maxidentlen", 1, 100);
		} else {
			unsigned long tmp;
			swscanf(strng, L"%u", &tmp);
			if (tmp < 1 || tmp > 100) {
				swprintf(temp, rs::rs(1332), L"maxidentlen", 1, 100);
			} else {
				Automap_MaxIdenticalExits = tmp;
				swprintf(temp, rs::rs(1333), L"maxidentlen", Automap_MaxIdenticalExits);
			}
		}

		if (mesvar[MSG_MAPPER])
			tintin_puts2(temp);
	} else {
		tintin_puts2(rs::rs(1334));
	}
}

static void mapper_reset(wchar_t *arg)
{
	wchar_t obj[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, obj, sizeof(obj) / sizeof(wchar_t) - 1);

	if (is_abrev(obj, L"flag")) {
		CMapRoom *room = room_arg(&arg, L"reset flag");
		if (room) {
			wstring flag = arg_with_vars(&arg);
			room->delFlag(flag);
			if (mesvar[MSG_MAPPER]) {
				swprintf(temp, rs::rs(1326), flag.c_str(), room->ID().toString().c_str());
				tintin_puts2(temp);
			}
		}
	} else {
		tintin_puts2(rs::rs(1328));
	}		
}

static void mapper_get(wchar_t *arg)
{
	wchar_t obj[BUFFER_SIZE], strng[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, obj, sizeof(obj) / sizeof(wchar_t) - 1);

	if (is_abrev(obj, L"flag") || is_abrev(obj, L"exit")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);
	}

	RoomID id = room_id_arg(&arg, L"get");
	CMapRoom *room = findRoom(id);
	wstring value = L"0";
	if (room) {
		wchar_t buf[16];
		int n;
		//#mapper get id|name|descr|area|comment|flags|flag%d|exits|exit%d|flag <name>|exit <name> <id> <varname>
		if (is_abrev(obj, L"id")) {
			value = room->ID().toString();
		} else if (is_abrev(obj, L"name")) {
			value = room->name();
		} else if (is_abrev(obj, L"descr")) {
			value = room->descr();
		} else if (is_abrev(obj, L"area")) {
			value = room->area();
		} else if (is_abrev(obj, L"comment")) {
			value = room->comment();
		} else if (swscanf(obj, L"flag%u", &n) == 1 && n > 0) {
			set<wstring> flags = room->getFlags();
			if (n <= flags.size()) {
				set<wstring>::iterator it_flag = flags.begin();
				for (int i = 1; i < n; i++)
					it_flag++;
				value = *(it_flag);
			}
		} else if (is_abrev(obj, L"flag")) {
			set<wstring> flags = room->getFlags();
			if (flags.find(strng) != flags.end())
				value = L"1";
		} else if (is_abrev(obj, L"flags")) {
			swprintf(buf, L"%d", room->getFlags().size());
			value = buf;
		} else if (swscanf(obj, L"exit%u", &n) == 1 && n > 0) {
			set<CMapExit> exits = room->exits();
			if (n <= exits.size()) {
				set<CMapExit> exits = room->exits();
				set<CMapExit>::const_iterator it_exit = exits.begin();
				for (int i = 1; i < n; i++)
					it_exit++;
				value = it_exit->command();
			}
		} else if (is_abrev(obj, L"exit")) {
			wstring command = strng;
			if (SynonymToDirection.find(command) != SynonymToDirection.end())
				command = SynonymToDirection[command];

			set<CMapExit> exits = room->exits();
			set<CMapExit>::iterator it_exit;
			for (it_exit = exits.begin(); it_exit != exits.end(); it_exit++)
				if (it_exit->command() == command) {
					value = it_exit->toID().toString();
					break;
				}
		} else if (is_abrev(obj, L"exits")) {
			swprintf(buf, L"%d", room->exits().size());
			value = buf;
		} else {
			tintin_puts2(rs::rs(1329));
		}
	}
	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	set_variable(temp, value.c_str());
}

static void mapper_search(wchar_t *arg)
{
	wchar_t strng[BUFFER_SIZE], temp[BUFFER_SIZE];

	RoomID nearid(0, 0);
	int skip_count = 0;
		
	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);

	if (is_abrev(strng, L"near")) {
		nearid = room_id_arg(&arg, L"search");
		if (nearid == RoomID(0, 0))
			return;
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);
	}

	if (is_all_digits(strng)) {
		swscanf(strng, L"%u", &skip_count);

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);
	}

	vector <CRoomFilter> filters;

	while (arg[0] != L'\0') {
		wstring key = strng;

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);

		wstring value = strng;

		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);

		CRoomFilter filter(key.c_str(), value.c_str());
		if (!filter.valid()) {
			swprintf(temp, rs::rs(1336), key.c_str());
			tintin_puts2(temp);
			return;
		}

		filters.push_back(filter);
	}

	RoomID found(0, 0);

	vector < CRoomFilter >::const_iterator it_filter;

	if (nearid == RoomID(0, 0)) {
		map < unsigned long, map <unsigned long, CMapRoom*> >::const_iterator it_room1;
		map < unsigned long, CMapRoom*>::const_iterator it_room2;

		for (it_room1 = Rooms.begin(); it_room1 != Rooms.end(); it_room1++) {
			for (it_room2 = it_room1->second.begin(); it_room2 != it_room1->second.end(); it_room2++) {
				bool ok = true;
				for (it_filter = filters.begin(); it_filter != filters.end(); it_filter++) {
					if (!it_filter->check(it_room2->second)) {
						ok = false;
						break;
					}
				}
				if (ok && skip_count-- == 0) {
					found = RoomID(it_room1->first, it_room2->first);
					break;
				}
			}
			if (!(found == RoomID(0, 0)))
				break;
		}
	} else {
		set < pair<unsigned long, RoomID> > q;
		q.insert( pair<unsigned long, RoomID>(0, nearid) );
			
		set <RoomID> seen;
		while (q.size() > 0) {
			unsigned long dst = q.begin()->first;
			RoomID next = q.begin()->second;
			q.erase(q.begin());

			if (seen.find(next) != seen.end())
				continue;
			seen.insert(next);

			CMapRoom *room = findRoom(next);
			if (!room)
				continue;

			bool ok = true;
			for (it_filter = filters.begin(); it_filter != filters.end(); it_filter++) {
				if (!it_filter->check(room)) {
					ok = false;
					break;
				}
			}

			if (ok && skip_count-- == 0) {
				found = next;
				break;
			}

			set<CMapExit> exits = room->exits();
			set<CMapExit>::const_iterator it_exit;

			for (it_exit = exits.begin(); it_exit != exits.end(); it_exit++) {
				RoomID to = it_exit->toID();
				if (seen.find(to) != seen.end())
					continue;
				CMapRoom *to_room = findRoom(to);
				if (!to_room)
					continue;
				if (to_room->avoidance() <= 0)
					continue;
				q.insert( pair<unsigned long, RoomID>(dst + to_room->avoidance(), to) );
			}
		}
	}

	set_variable(temp, found.toString().c_str());
}

static void mapper_path(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE];

	RoomID fromid = room_id_arg(&arg, L"path");
	if (fromid == RoomID(0, 0))
		return;
	RoomID toid = room_id_arg(&arg, L"path");
	if (toid == RoomID(0, 0))
		return;

	static RoomID prev_fromid(0, 0);
	static RoomID prev_toid(0, 0);
	static vector< pair<wstring, RoomID> > prev_path;

	bool prev_ok = false;
	if (prev_toid == toid) {
		if (prev_fromid == fromid) {
			prev_ok = true;
		} else if (prev_path.size() > 0 && prev_path[0].second == fromid) {
			prev_fromid = prev_path[0].second;
			prev_path.erase(prev_path.begin());
			prev_ok = true;
		}
	}

	if (!prev_ok) {
		prev_path.clear();
		prev_fromid = fromid;
		prev_toid = toid;

		set < pair<unsigned long, RoomID> > q;
		q.insert( pair<unsigned long, RoomID>(0, fromid) );
			
		map <RoomID, unsigned long> dists;
		while (q.size() > 0) {
			unsigned long dst = q.begin()->first;
			RoomID next = q.begin()->second;
			q.erase(q.begin());

			if (dists.find(next) != dists.end())
				continue;
			dists[next] = dst;

			CMapRoom *room = findRoom(next);
			if (!room)
				continue;

			if (next == toid) {
				break;
			}
				
			set<CMapExit> exits = room->exits();
			set<CMapExit>::const_iterator it_exit;

			for (it_exit = exits.begin(); it_exit != exits.end(); it_exit++) {
				RoomID to = it_exit->toID();
				if (dists.find(to) != dists.end())
					continue;
				CMapRoom *to_room = findRoom(to);
				if (!to_room)
					continue;
				if (to_room->avoidance() <= 0)
					continue;
				q.insert( pair<unsigned long, RoomID>(dst + to_room->avoidance(), to) );
			}
		}

		if (dists.find(toid) != dists.end()) {
			while (!(toid == fromid)) {
				int dst = dists[toid];
				CMapRoom *room = findRoom(toid);
				dst -= room->avoidance();
					
				set<CMapExit> entrances = room->entrances();
				set<CMapExit>::const_iterator it_entrance;
				bool ok = false;
				for (it_entrance = entrances.begin(); it_entrance != entrances.end(); it_entrance++) {
					RoomID from = it_entrance->fromID();
					if (dists.find(from) == dists.end())
						continue;
					if (dists[from] == dst) {
						CMapRoom *room = findRoom(from);
						if (room) {
							CMapExit exit = room->getExit(it_entrance->command());
							wstring cmd = exit.passCommand();
							if (cmd.length() > 0)
								cmd += cCommandDelimiter;
							cmd += exit.command();
							prev_path.insert(prev_path.begin(), pair<wstring, RoomID>(cmd, toid));
							toid = from;
							ok = true;
							break;
						}
					}
				}

				if (!ok) {
					prev_toid = prev_fromid = RoomID(0, 0);
					prev_path.clear();
					tintin_puts2(rs::rs(1337));
				}
			}
		}
	}

	if (!bPathing) {
		kill_list(common_path);
		common_path=init_list();
		path_length=0;
		bPathing = TRUE;
		if (prev_path.size() > 0) {
			int i;
			for (i = 0; i < prev_path.size(); i++)
				check_insert_path((wchar_t*)prev_path[i].first.c_str());
		}
		bPathing = FALSE;
	}

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	if (prev_path.size() > 0)
		set_variable(temp, prev_path[0].first.c_str());
	else
		set_variable(temp, L"");
}

static void mapper_print(wchar_t *arg)
{
	wchar_t strng[BUFFER_SIZE], temp[BUFFER_SIZE];

	RenderStyle style = RenderNormal;
	int width = 80, height = 24;
	bool use_colors = true;
	bool html = false;
	bool cropped = false;
	bool maxsize = true;
	int flags = 0;
	int wnd = -1;
	RoomID id = RoomID(0,0);
	int tmp1, tmp2;
	wstring fname = L"";

	for(;;) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
		prepare_actionalias(temp, strng, sizeof(strng) / sizeof(wchar_t) - 1);
		if (!strng[0])
			break;

		if (is_abrev(strng, L"tiny"))
			style = RenderTiny;
		else if (is_abrev(strng, L"normal"))
			style = RenderNormal;
		else if (is_abrev(strng, L"full"))
			style = RenderFull;
		else if (is_abrev(strng, L"nocolors"))
			use_colors = false;
		else if (is_abrev(strng, L"html"))
			html = true;
		else if (is_abrev(strng, L"crop"))
			cropped = true;
		else if (is_abrev(strng, L"exits"))
			flags |= (int)RenderPrintExits;
		else if (is_abrev(strng, L"header"))
			flags |= (int)RenderPrintHeader;
		else if (is_abrev(strng, L"description"))
			flags |= (int)RenderPrintDescr;
		else if (is_abrev(strng, L"flags"))
			flags |= (int)RenderPrintFlags;
		else if (is_abrev(strng, L"comment"))
			flags |= (int)RenderPrintComment;
		else if (is_abrev(strng, L"main"))
			wnd = -1;
		else if (swscanf(strng, L"%ux%u", &tmp1, &tmp2) == 2) {
			width = tmp1;
			height = tmp2;
			maxsize = false;
		}
		else if (swscanf(strng, L"w%u", &tmp1) == 1)
			wnd = tmp1;
		else if (RoomID::fromString(strng, id))
		{
		}
		else {
			wnd = -2;
			fname = strng;
		}
	}

	if (id == RoomID(0, 0) && Rooms.size() > 0) {
		map <unsigned long, CMapRoom*> lst = Rooms.begin()->second;
		if (lst.size() > 0)
			id = lst.begin()->second->ID();
	}
	
	if (maxsize && wnd != -2) {
		GetWindowSize(wnd, width, height);
		if (width > 1) //to guarantee non-wrapping
			width--;
	}
		
	vector<wstring> map = render_map(id, width, height, use_colors, cropped, style, flags);
		
	if (wnd == -2) {
		wstring contents = L"";

		if (html) {
			//wstring html_header = loadHTMLFromFile(L"html.log.template");
			wstring html_header = loadHTMLFromResource(IDR_HTML_HEAD);
			wstring::size_type pos = 0;
			while ( ( pos = html_header.find(L"%title%", pos) ) != wstring::npos )
				html_header.replace(pos, 7, L"Map");
			pos = 0;
			while ( ( pos = html_header.find(L"%charset%", pos) ) != wstring::npos )
				html_header.replace(pos, 9, L"utf-8");
			contents += html_header;
		}

		for (int i = 0; i < map.size(); i++) {
			if (html)
				contents += processHTML(map[i], 1);
			else
				contents += map[i];
			contents += L'\n';
		}

		if (html)
			contents += html_footer;

		write_file_contents(fname.c_str(), contents.c_str(), contents.length());
	} else if (wnd == -1) {
		for (int i = 0; i < map.size(); i++)
			tintin_puts2(map[i].c_str());
	} else {
		for (int i = 0; i < map.size(); i++)
			tintin_puts3(map[i].c_str(), wnd);
	}
}

void mapper_command(wchar_t *arg)
{
	wchar_t act[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, act, STOP_SPACES, sizeof(act) / sizeof(wchar_t) - 1);

	if (is_abrev(act, L"clear")) {
		mapper_clear();
	} else if (is_abrev(act, L"write")) {
		mapper_write(arg);
	} else if (is_abrev(act, L"track")) {
		mapper_track(arg);
	} else if (is_abrev(act, L"autoid")) {
		mapper_autoid(arg);
	} else if (is_abrev(act, L"del")) {
		mapper_del(arg);
	} else if (is_abrev(act, L"merge")) {
		mapper_merge(arg);
	} else if (is_abrev(act, L"add")) {
		mapper_add(arg);
	} else if (is_abrev(act, L"set")) {
		mapper_set(arg);
	} else if (is_abrev(act, L"reset")) {
		mapper_reset(arg);
	} else if (is_abrev(act, L"get")) {
		mapper_get(arg);
	} else if (is_abrev(act, L"search")) {
		mapper_search(arg);
	} else if (is_abrev(act, L"path")) {
		mapper_path(arg);
	} else if (is_abrev(act, L"print")) {
		mapper_print(arg);
	} else {
		tintin_puts2(rs::rs(1335));
	}
}
