#pragma once

#include <utility>

namespace Fiberz::Internal {

/* This class creates a couple of paired instances. The master one, instantiated from the main class, and a slave,
 * that can be stored elsewhere.
 *
 * The slave instance can be moved around, and the master will keep track of where the slave is. As long as the master
 * lives, the slave can be reached and changed.
 */
template<typename T>
class EPR {
    class Slave;

    Slave *slave_ = nullptr;

    class Slave {
        friend EPR;

        EPR *master_;
        T element_;

    public:
        template<typename... Args>
        explicit Slave(EPR *master, Args&&... args) : master_(master), element_(std::forward<Args>(args)...) {}

        Slave(const Slave &that) = delete;
        Slave &operator=(const Slave &that) = delete;

        Slave(Slave &&that) : master_(that.master_), element_(std::move(that.element_)) {
            if( master_ != nullptr ) {
                assert( master_->slave_ == &that );
                master_->slave_ = this;
            }
        }
        Slave &operator=(Slave &&that) {
            if( this != &that ) {
                clear();

                master_ = that.master_;
                element_ = std::move(that.element_);

                if( master_ != nullptr ) {
                    assert( master_->slave_ == &that );
                    master_->slave_ = this;
                }
            }

            return *this;
        }

        ~Slave() {
            clear();
        }

        void clear() {
            if( master_ != nullptr ) {
                assert( master_->slave_ == this );
                master_->slave_ = nullptr;
            }

            master_ = nullptr;
        }
    };

public:
    EPR() = default;

    EPR(const EPR &that) = delete;
    EPR &operator=(const EPR &that) = delete;
    EPR(EPR &&that) : slave_(that.slave_) {
        if( slave_ != nullptr ) {
            assert(slave_->master_ == &that);
            slave_->master = this;
            that.slave_ = nullptr;
        }
    }
    EPR &operator=(EPR &&that) {
        if( this != &that ) {
            clear();

            if( slave_ != nullptr ) {
                slave_ = that.slave_;
                that.slave_ = nullptr;

                assert(slave_->master == &that);
                slave_->master = this;
            }
        }

        return *this;
    }

    ~EPR() { clear(); }

    void clear() {
        if( slave_ != nullptr ) {
            assert( slave_->master_ == this);
            slave_->master_ = nullptr;
            slave_ = nullptr;
        }
    }

    template<typename... Args>
    Slave createSlave(Args&&... args) {
        assert( slave_ == nullptr );

        auto slave = Slave( this, std::forward<Args>(args)... );
        slave_ = &slave;

        return slave;
    }
};

} // namespace Fiberz::Internal
