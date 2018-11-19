/** Copyright &copy; 2016, Alfarobotics.
 * \brief  Контроллер организации комнат.
 * \author Величко Ростислав
 * \date   12.10.2016
 */

#include <tuple>
#include <string>
#include <map>
#include <set>



namespace utils {


/**
 *  Базовый класс контроллера комнат, определяет общие методы управления комнатами для 2 классов мемберов
 */
template<class TRoomMembers>
class BaseRoomController {
public:
    typedef typename std::map<std::string, TRoomMembers> MapRooms;
    typedef typename MapRooms::iterator MapRoomIter;

protected:
    MapRooms _rooms;
    TRoomMembers _null_members;

public:
    virtual ~BaseRoomController()
    {}

    /**
     *  Метод поиска комнаты по идентификатору
     *
     * \param room_id  Идентификатор искомой комнаты
     */
    TRoomMembers getRoom(const std::string &room_id) {
        TRoomMembers members = _null_members;
        MapRoomIter iter = _rooms.find(room_id);
        if (iter not_eq _rooms.end()) { ///< При наличии искомой комнаты проинициализировать возвращаемую структуру комнаты
            members = iter->second;
        }
        return members;
    }

    /**
     *  Метод удаления комнаты по идентификатору
     *
     * \param room_id  Идентификатор искомой комнаты
     */
    void deleteRoom(size_t room_id) {
        _rooms.erase(room_id);
    }

    /**
     *  Шаблонный метод установки данных в структуру комнаты
     *
     * \param room_id  Идентификатор искомой комнаты
     * \param room_id  Шаблонный тип хранимых данных
     */
    template<class TDataType>
    bool setRoomData(const std::string &room_id, const TDataType &room_data) {
        bool res = false;
        auto iter = _rooms.find(room_id);
        if (iter not_eq _rooms.end()) {
            std::get<2>(iter->second) = room_data;
        }
        return res;
    }

    /**
     *  Возврат копии структуры комнат
     */
    MapRooms getRooms() {
        MapRooms rooms = _rooms;
        return rooms;
    }
};


/**
 *  Шаблонная структура - хелпер, для определения неопределённых шаблонных типов структур комнаты
 */
template<class TDataType>
struct RoomData {
    typedef std::tuple<size_t, size_t, TDataType> SingleMembers; ///< Комната 1 : 1
    typedef std::tuple<size_t, std::set<size_t>, TDataType> SingleRobotMembers; ///< Комната 1 : N
    typedef std::tuple<std::set<size_t>, std::set<size_t>, TDataType> MultyMembers; ///< Комната N : N
};

typedef std::tuple<size_t, size_t> Members; ///< Тип управления идентификаторами мемберов комнаты


/**
 *  Шаблонный класс, комнаты 1 : 1
 */
template<class TDataType, class TRoomMembers = typename RoomData<TDataType>::SingleMembers>
class RoomController
    : public BaseRoomController<TRoomMembers> {
    typedef BaseRoomController<TRoomMembers> Base;

public:
    RoomController() {
        std::get<0>(Base::_null_members) = 0;
        std::get<1>(Base::_null_members) = 0;
    }

    virtual ~RoomController()
    {}

    TRoomMembers addToRoom(const std::string &room_id, const Members &mbr) {
        TRoomMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second) = id_0;
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second) = id_1;
            }
            members = iter->second;
        } else {
            std::get<0>(members) = std::get<0>(mbr);
            std::get<1>(members) = std::get<1>(mbr);
            Base::_rooms.insert(std::make_pair(room_id, members));
        }
        return members;
    }

    TRoomMembers deleteFromRoom(const std::string &room_id, const Members &mbr) {
        TRoomMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second) = 0;
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second) = 0;
            }
            members = iter->second;
        }
        return members;
    }
};


/**
 *  Шаблонный класс, комнаты 1 : N
 */
template<class TDataType>
class RoomController<TDataType, typename RoomData<TDataType>::SingleRobotMembers>
    : public BaseRoomController<typename RoomData<TDataType>::SingleRobotMembers> {
    typedef BaseRoomController<typename RoomData<TDataType>::SingleRobotMembers> Base;

public:
    RoomController() {
        std::get<0>(Base::_null_members) = 0;
    }

    virtual ~RoomController()
    {}

    typename RoomData<TDataType>::SingleRobotMembers addToRoom(const std::string &room_id, const Members &mbr) {
        typename RoomData<TDataType>::SingleRobotMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second) = id_0;
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second).insert(id_1);
            }
            members = iter->second;
        } else {
            std::get<0>(members) = std::get<0>(mbr);
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(members) = std::set<size_t>({id_1});
            }
            Base::_rooms.insert(std::make_pair(room_id, members));
        }
        return members;
    }

    typename RoomData<TDataType>::SingleRobotMembers deleteFromRoom(const std::string &room_id, const Members &mbr) {
        typename RoomData<TDataType>::SingleRobotMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second) = 0;
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second).erase(id_1);
            }
            members = iter->second;
        }
        return members;
    }
};


/**
 *  Шаблонный класс, комнаты N : N
 */
template<class TDataType>
class RoomController<TDataType, typename RoomData<TDataType>::MultyMembers>
    : public BaseRoomController<typename RoomData<TDataType>::MultyMembers> {
    typedef BaseRoomController<typename RoomData<TDataType>::MultyMembers> Base;

public:
    RoomController()
    {}

    virtual ~RoomController()
    {}

    typename RoomData<TDataType>::MultyMembers addToRoom(const std::string &room_id, const Members &mbr) {
        typename RoomData<TDataType>::MultyMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second).insert(id_0);
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second).insert(id_1);
            }
            members = iter->second;
        } else {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(members) = std::set<size_t>({id_0});
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(members) = std::set<size_t>({id_1});
            }
            Base::_rooms.insert(std::make_pair(room_id, members));
        }
        return members;
    }

    typename RoomData<TDataType>::MultyMembers deleteFromRoom(const std::string &room_id, const Members &mbr) {
        typename RoomData<TDataType>::MultyMembers members;
        auto iter = Base::_rooms.find(room_id);
        if (iter not_eq Base::_rooms.end()) {
            size_t id_0 = std::get<0>(mbr);
            if (id_0 not_eq 0) {
                std::get<0>(iter->second).erase(id_0);
            }
            size_t id_1 = std::get<1>(mbr);
            if (id_1 not_eq 0) {
                std::get<1>(iter->second).erase(id_1);
            }
            members = iter->second;
        }
        return members;
    }
};
} // utils

