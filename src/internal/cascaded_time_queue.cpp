#include <fiberz/internal/cascaded_time_queue.h>

namespace Fiberz::Internal {

CascadedTimeQueue::TimerHandle::~TimerHandle() {
    if( queue_ )
        removeEvent();
}

void CascadedTimeQueue::TimerHandle::removeEvent() {
    assert( queue_ != nullptr );
    queue_->removeEvent( *this );
    queue_ = nullptr;
}

void CascadedTimeQueue::TimerHandle::release() {
    event_ = nullptr;
    queue_ = nullptr;
}

void CascadedTimeQueue::TimerHandle::call() const {
    assert( !isEmpty() );

    event_->callback();
}

TimePoint CascadedTimeQueue::TimerHandle::getTime() const {
    assert( !isEmpty() );

    return event_->expires;
}

CascadedTimeQueue::CascadedTimeQueue( TimePoint current_time, size_t numLevels, Duration resolution ) :
    cascaded_list_( numLevels ),
    start_( current_time ),
    ticks_count_(0),
    resolution_(resolution)
{}

void CascadedTimeQueue::removeEvent( const TimerHandle &handle ) {
    removeEvent( handle.event_.get() );
}

TimePoint CascadedTimeQueue::nextEvent() {
    if( next_event_tick_==NextEventUnknown )
        recalcNextEvent();

    if( next_event_tick_==NoNextEvent )
        return TimePoint::max();
    else
        return start_ + resolution_*next_event_tick_;
}

CascadedTimeQueue::TimerHandle CascadedTimeQueue::expiredEvent( TimePoint now ) {
    advanceTime( now );

    ListType &list = currentBucket();

    if( !list.empty() ) {
        TimerHandle ret( list.front() );
        list.pop_front();

        if( list.empty() )
            next_event_tick_ = NextEventUnknown;

        return ret;
    } else {
        return TimerHandle();
    }
}

// Private
void CascadedTimeQueue::recalcNextEvent() {
    long ticksPerCell = 1;
    size_t level_idx = 0;

    for( const auto &level : cascaded_list_ ) {
        long tick = firstTickOfLevel( level_idx );
        for( const auto &list : level ) {
            if( ! list.empty() ) {
          next_event_tick_ = tick;
                return;
            }

            tick += ticksPerCell;
        }

        ticksPerCell *= NodesPerLevel;
        level_idx++;
    }

    next_event_tick_ = NoNextEvent;
}

long CascadedTimeQueue::firstTickOfLevel(size_t level) const {
    long level_span = NodesPerLevel;
    for( size_t i=0; i<level; ++i ) {
        level_span *= NodesPerLevel;
    }

    long aligned_ticks = ticks_count_ / level_span;
    aligned_ticks *= level_span;

    return aligned_ticks;
}

CascadedTimeQueue::ListType &CascadedTimeQueue::currentBucket() {
    size_t index = ticks_count_ % NodesPerLevel;

    return cascaded_list_[0][index];
}

void CascadedTimeQueue::advanceTime( TimePoint tp ) {
    const long time_point_ticks = timePointToTicks( tp );

    if( time_point_ticks<ticks_count_ )
        return;

    if( time_point_ticks<next_event_tick_ ) {
        // We're not advancing enough to catch up to the next event
        // NoNextEvent is minimal value, so we won't enter this clause if that's the value
        ticks_count_ = time_point_ticks;

        return;
    }

    size_t step = 1, mask = NodesPerLevel;
    size_t level = 0, level_idx = ticks_count_ % mask;

    while( ticks_count_ < time_point_ticks && cascaded_list_[level][level_idx].empty() ) {
        ticks_count_ += step;
        level_idx = ticks_count_ % mask / step;

        if( level_idx != 0 )
            continue;   // Fast path

        // Start from the next level as we already know we finished the first one
        do {
            level++;
            step = mask;
            mask *= NodesPerLevel;
            level_idx = ticks_count_ % mask / step;

            assert( level < cascaded_list_.size() );    // We finished the time queue
        } while( ticks_count_ % mask == 0 );
    }

    if( level>0 ) { // Acts as recursion termination condition
        // Distribute the events in this node to the relevant lower level nodes
        ListType events( std::move( cascaded_list_[level][level_idx] ) );

        while( !events.empty() ) {
            auto front = events.front();
            events.pop_front();
            front->owner = nullptr;
            insert( std::move(front), level );

            next_event_tick_ = NextEventUnknown;
        }

        advanceTime( tp );
    }
}

void CascadedTimeQueue::insert( ListType::NodePtr event, size_t max_level ) {
    assert( ! event->owner );

    long event_tick = timePointToTicks( event->expires );
    if( event_tick < ticks_count_ )
        event_tick = ticks_count_;

    // Find the level into which this event needs to go
    size_t level = 0;
    size_t step = 1;

    long level_start = firstTickOfLevel(level);
    long level_end = level_start + step*NodesPerLevel;
    while( event_tick >= level_end ) {
        level += 1;
        assert( level<max_level );

        step *= NodesPerLevel;

        level_start = firstTickOfLevel(level);
        level_end = level_start + step*NodesPerLevel;
    }

    const size_t level_idx = (event_tick - level_start) / step;

    event->owner = &cascaded_list_[level][level_idx];

    cascaded_list_[level][level_idx].push_back( std::move(event) );

    if( event_tick < next_event_tick_ )
        next_event_tick_ = event_tick;
}

void CascadedTimeQueue::insertEvent( TimePoint expiery, Callback callback ) {
    insertEventWithHandle( expiery, std::move(callback) ).release();
}

CascadedTimeQueue::TimerHandle CascadedTimeQueue::insertEventWithHandle( TimePoint expiery, Callback callback ) {
    ListType::NodePtr event( new TimerEvent( expiery, std::move(callback) ) );

    insert( event, cascaded_list_.size() );

    return TimerHandle( std::move( event ) );
}

void CascadedTimeQueue::removeEvent( TimerEvent *event ) {
    assert( event->owner != nullptr );

    event->owner->erase( event );

    long event_ticks = timePointToTicks( event->expires );
    if( event_ticks == next_event_tick_ && event->owner->empty() )
        next_event_tick_ = NextEventUnknown;

    event->owner = nullptr;
}

long CascadedTimeQueue::timePointToTicks( TimePoint tp ) const {
    auto sinceStart = tp - start_;
    // Round up
    sinceStart += resolution_;
    sinceStart -= decltype(resolution_)(1);

    return sinceStart / resolution_;
}

} // namespace Fiberz::Internal
