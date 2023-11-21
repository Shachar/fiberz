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
    next_event_tick_( NoNextEvent ),
    resolution_( resolution ),
    first_occupied_level_( numLevels )
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
    long step = 1;

    long tick = ticks_count_;

    for( size_t level_num = 0; level_num<cascaded_list_.size(); level_num++ ) {
        long level_start = firstTickOfLevel( level_num );
        long level_end = level_start + NodesPerLevel*step;

        if( level_num>=first_occupied_level_ ) {
            unsigned level_idx = tick % (step * NodesPerLevel) / step;
            if( level_num!=0 && level_idx==0 )
                level_idx=NodesPerLevel;

            for( ; level_idx<NodesPerLevel; level_idx++ ) {
                auto &list = cascaded_list_[level_num][level_idx];
                if( ! list.empty() ) {
                    next_event_tick_ = level_start + step * level_idx;
                    return;
                }
            }

            tick = level_end;
            first_occupied_level_ = level_num+1;
        } else {
            tick = level_end;
        }

        step *= NodesPerLevel;
        assert( tick == level_end );
    }

    assert( first_occupied_level_ == cascaded_list_.size() );

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
    size_t level_num = 0;

    while( level_num < first_occupied_level_ ) {
        level_num++;
        step = mask;
        mask *= NodesPerLevel;
    }

    size_t level_idx = ticks_count_ % mask;
    // Round up
    level_idx += step - 1;
    level_idx /= step;

    ticks_count_ = firstTickOfLevel(level_num) + level_idx * step;
    assert( ticks_count_<=time_point_ticks );

    while( ticks_count_ < time_point_ticks && cascaded_list_[level_num][level_idx].empty() ) {
        ticks_count_ += step;
        level_idx++;

        if( level_idx != 0 )
            continue;   // Fast path

        // Start from the next level as we already know we finished the first one
        do {
            level_num++;
            step = mask;
            mask *= NodesPerLevel;
            level_idx = ticks_count_ % mask / step;

            assert( level_num < cascaded_list_.size() );    // We finished the time queue
        } while( ticks_count_ % mask == 0 );
    }

    if( !cascaded_list_[level_num][level_idx].empty() )
        first_occupied_level_ = level_num;

    if( level_num>0 ) { // Acts as recursion termination condition
        // Distribute the events in this node to the relevant lower level nodes
        ListType events( std::move( cascaded_list_.at(level_num).at(level_idx) ) );

        if( !events.empty() ) {
            // The events we're currently distributing are, by definition, the earliest in the array. By marking
            // the next event NoNextEvent, we make sure it's coorectly set after redistribution
            next_event_tick_ = NoNextEvent;
        }

        while( !events.empty() ) {
            auto front = events.front();
            events.pop_front();
            front->owner = nullptr;

            insert( std::move(front), level_num );

            assert( first_occupied_level_<level_num );
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
    size_t level_num = 0;
    size_t step = 1;

    long level_start = firstTickOfLevel(level_num);
    long level_end = level_start + step*NodesPerLevel;
    while( event_tick >= level_end ) {
        level_num += 1;
        assert( level_num<max_level );

        step *= NodesPerLevel;

        level_start = firstTickOfLevel(level_num);
        level_end = level_start + step*NodesPerLevel;
    }

    const size_t level_idx = (event_tick - level_start) / step;

    event->owner = &cascaded_list_[level_num][level_idx];

    cascaded_list_[level_num][level_idx].push_back( std::move(event) );

    if( event_tick < next_event_tick_ )
        next_event_tick_ = event_tick;

    if( level_num < first_occupied_level_ )
        first_occupied_level_ = level_num;
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
