#pragma once

#include <sys/inotify.h>
#include <errno.h>

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/assign.hpp>

#include "../file_monitor_event.hpp"

namespace services { namespace detail {

    class file_monitor_service
    {
        public:
            class implementation_type :
                private boost::asio::detail::noncopyable
            {
                int fd_;
                std::map< int, std::string > watched_files_;
                boost::shared_ptr< boost::asio::posix::stream_descriptor > input_;
                boost::array<char, 4096> buffer_;
                std::string buffer_str_;

                friend class file_monitor_service;
            };

            explicit file_monitor_service ( boost::asio::io_service& io_service ) :
                io_service_( io_service )
            {
            }

            void shutdown_service ()
            {
            }

            void construct( implementation_type& impl )
            {
                impl.fd_ = init_fd();
                impl.input_.reset( new boost::asio::posix::stream_descriptor( io_service_, impl.fd_ ) );
            }

            void destroy( implementation_type& impl )
            {
                impl.input_.reset();
            }

            void add_file ( implementation_type& impl, const std::string& file, boost::system::error_code& ec )
            {
                int wd = inotify_add_watch( impl.fd_, file.c_str(), IN_ALL_EVENTS );

                if (wd == -1)
                {
                    ec = boost::system::error_code(errno, boost::system::get_system_category());
                }
                else if( impl.watched_files_.find( wd ) == impl.watched_files_.end() )
                {
                    impl.watched_files_[wd] = file;
                }
            }

            template <typename MonHandler>
                void async_monitor ( implementation_type& impl, boost::system::error_code& ec, MonHandler handler )
                {
                    impl.input_->async_read_some
                        (
                         boost::asio::buffer( impl.buffer_ ),
                         boost::bind
                         (
                          &file_monitor_service::handle_monitor<MonHandler>, this, boost::ref( impl ),
                          boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, handler
                         )
                        );
                }

        private:
            template <typename MonHandler>
                void handle_monitor ( implementation_type& impl, boost::system::error_code ec, std::size_t bytes_transferred, MonHandler handler )
                {
                    if( !ec )
                    {
                        impl.buffer_str_.append( impl.buffer_.data(), impl.buffer_.data() + bytes_transferred );

                        while( impl.buffer_str_.size() >= sizeof( inotify_event ) )
                        {
                            const inotify_event *iev = reinterpret_cast<const inotify_event *>( impl.buffer_str_.data() );
                            events_t::const_iterator event_i = events.find( iev->mask );
                            if( event_i != events.end() )
                            {
                                io_service_.post
                                    (
                                     boost::asio::detail::bind_handler
                                     (
                                      handler, ec,
                                      event
                                      (
                                       impl.watched_files_[iev->wd],
                                       event_i->second
                                      )
                                     )
                                    );
                            }
                            impl.buffer_str_.erase( 0, sizeof( inotify_event ) + iev->len );
                        }
                        async_monitor( impl, ec, handler );
                    }
                }

            int init_fd ()
            {
                int fd = inotify_init1( IN_NONBLOCK );
                if ( fd == -1 )
                {
                    boost::system::system_error e
                        (
                         boost::system::error_code( errno, boost::system::get_system_category() ),
                         "file_monitor_service::init_fd: init_inotify failed"
                        );
                    boost::throw_exception(e);
                }
                return fd;
            }

            boost::asio::io_service& io_service_;

            typedef std::map< int, event::event_type > events_t;
            static const events_t events;
    };

    const file_monitor_service::events_t file_monitor_service::events =
        boost::assign::map_list_of
        ( IN_ACCESS, event::access )
        ( IN_ATTRIB, event::attrib )
        ( IN_CLOSE_WRITE, event::close_write )
        ( IN_CLOSE_NOWRITE, event::close_nowrite )
        ( IN_MODIFY, event::modify )
        ( IN_DELETE_SELF, event::delete_self )
        ( IN_MOVE_SELF, event::move_self )
        ( IN_OPEN, event::open );
} }
