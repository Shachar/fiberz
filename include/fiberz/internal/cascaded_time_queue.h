#pragma once

#include <fiberz/containers/compact_intrusive_list.h>
#include <fiberz/internal/callable.h>

#include <fiberz/time.h>

namespace Fiberz::Internal {

class CascadedTimeQueue {
public:
    using Callback = ::Fiberz::Internal::Callable<void, TimePoint>;

private:
    static constexpr size_t NodesPerLevel = 256;

    struct TimerEvent;
    static constexpr size_t NodeOffsetInEvent = 0;
    using ListType = Containers::CompactIntrusiveList< TimerEvent, NodeOffsetInEvent >;

    struct TimerEvent {
        Containers::CompactIntrusiveList_Node node;
        TimePoint expires;
        Callback callback;
        mutable size_t ref_count = 0;
        ListType *owner = nullptr;

        explicit TimerEvent( TimePoint expiery, Callback callback ) : expires(expiery), callback(std::move(callback)) {}

        friend inline void intrusive_ptr_release(const TimerEvent *ptr) {
            if( --ptr->ref_count == 0 )
                delete ptr;
        }
        friend inline void intrusive_ptr_add_ref(const TimerEvent *ptr) {
            ptr->ref_count++;
        }
    };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    static_assert( offsetof( TimerEvent, node ) == NodeOffsetInEvent, "Node element in TimerEvent class must be at correct offset" );
#pragma GCC diagnostic pop

    static constexpr long NoNextEvent = std::numeric_limits<long>::max();
    static constexpr long NextEventUnknown = std::numeric_limits<long>::min();

    // Members
    std::vector<
            std::array<
                ListType,
                NodesPerLevel
            >
        > cascaded_list_;
    TimePoint start_;
    long ticks_count_ = 0, next_event_tick_ = NextEventUnknown;
    Duration resolution_;
    unsigned first_occupied_level_ = 0;

public:
    class TimerHandle {
        friend CascadedTimeQueue;

        boost::intrusive_ptr<TimerEvent> event_;
        CascadedTimeQueue *queue_ = nullptr;

    public:
        explicit TimerHandle() = default;
        explicit TimerHandle( boost::intrusive_ptr<TimerEvent> event, CascadedTimeQueue &queue ) :
            event_( std::move(event) ), queue_(&queue)
        {}

        TimerHandle( const TimerHandle &that ) = delete;
        TimerHandle &operator=( const TimerHandle &that ) = delete;

        TimerHandle( TimerHandle &&that ) = default;
        TimerHandle &operator=( TimerHandle &&that ) = default;

        ~TimerHandle();

        void removeEvent();
        void release();

        bool isEmpty() const { return !event_; }
        bool isValid() const;

        TimePoint getTime() const;
    };

    explicit CascadedTimeQueue( TimePoint current_time, size_t numLevels, Duration resolution = 1ms );

    CascadedTimeQueue( const CascadedTimeQueue &that ) = delete;
    CascadedTimeQueue &operator=( const CascadedTimeQueue &that ) = delete;

    void insertEvent( TimePoint expiery, Callback callback );
    [[nodiscard]] TimerHandle insertEventWithHandle( TimePoint expiery, Callback callback );
    [[nodiscard]] TimerHandle insertRecurringEvent( Duration period, Callback callback );
    void removeEvent( const TimerHandle &handle );

    TimePoint nextEvent();
    bool executeEvent( TimePoint now );

private:
    void recalcNextEvent();
    long firstTickOfLevel(size_t level_idx) const;

    ListType &currentBucket();
    void advanceTime( TimePoint tp );

    void insert( ListType::NodePtr event, size_t max_level );
    void removeEvent( TimerEvent *event );
    long timePointToTicks( TimePoint tp ) const;
};

} // namespace Fiberz::Internal
