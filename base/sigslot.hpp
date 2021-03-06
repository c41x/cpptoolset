/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: sigslot.h
 * created: 15-12-2012
 *
 * description: Singal-Slot implementation based on C++11 functional stuff
 *
 * changelog:
 * - 15-12-2012: file created
 * - 19-12-2012: first working version
 * - 19-01-2013: added asserts
 * - 29-09-2013: changelog moved to GIT repository
 *
 * notes:
 * - there are no return type because it does not make any sense in this case
 *   (we are invoking slots in loop)
 */

#pragma once

#include "includes.hpp"
#include "log.hpp"

namespace granite { namespace base {

namespace detail {
template<typename S, typename I, typename FN> struct bind {
    template<int N> static typename std::enable_if<N == 0>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref); }
    template<int N> static typename std::enable_if<N == 1>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1); }
    template<int N> static typename std::enable_if<N == 2>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2); }
    template<int N> static typename std::enable_if<N == 3>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); }
    template<int N> static typename std::enable_if<N == 4>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4); }
    template<int N> static typename std::enable_if<N == 5>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5); }
    template<int N> static typename std::enable_if<N == 6>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6); }
    template<int N> static typename std::enable_if<N == 7>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7); }
    template<int N> static typename std::enable_if<N == 8>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8); }
    template<int N> static typename std::enable_if<N == 9>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9); }
    template<int N> static typename std::enable_if<N == 10>::type get(S ptr, I *ref, FN &fp) { fp = std::bind(ptr, ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8,std::placeholders::_9, std::placeholders::_10); }
};
}

template<typename... P> class sig {
    static const int argsCount = sizeof...(P);
    typedef std::function<void(P...)> tdSlot;
    typedef std::list<tdSlot> tdSlots;
    tdSlots m_slots;

public:
    typedef typename tdSlots::iterator slotId;

    sig(){}
    ~sig(){}

    slotId connect(const tdSlot &f) {
        gassert(f, "passed std::function object is not valid");
        m_slots.push_front(f);
        return m_slots.begin();
    }

    template<typename MP, typename TP> slotId connect(MP mp, TP *tp) {
        tdSlot slot;
        detail::bind<MP,TP,tdSlot>::template get<argsCount>(mp, tp, slot);
        m_slots.push_front(slot);
        return m_slots.begin();
    }

    void disconnect(slotId id) {
        m_slots.erase(id);
    }

    void disconnectAll() {
        m_slots.clear();
    }

    void fire(P... args) {
        for(auto &it : m_slots) {
            gassert(it, "call to invalid slot");
            it(args...);
        }
    }

    slotId operator+=(const tdSlot &f) { return connect(f); }
    void operator-=(slotId id) { disconnect(id); }
};


template<typename... P> class delegate
{
    static const int argsCount = sizeof...(P);
    typedef std::function<void(P...)> tdSlot;
    tdSlot m_slot;

public:
    delegate(){}
    ~delegate(){}

    void connect(const tdSlot &f) {
        m_slot = f;
    }

    template<typename MP,typename TP> void connect(MP mp, TP *tp) {
        detail::bind<MP,TP,tdSlot>::template get<argsCount>(mp, tp, m_slot);
    }

    void fire(P... args) {
        m_slot(args...);
    }

    void operator=(const tdSlot &f) {
        return connect(f);
    }
};

}}
//~
