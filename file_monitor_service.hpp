#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
#  include "inotify/file_monitor_service.hpp"
#else
#  error "Platform not supported."
#endif

namespace services
{
    class file_monitor_service :
        public boost::asio::io_service::service
    {
        private:
            typedef detail::file_monitor_service service_impl_type;
        public:
            typedef service_impl_type::implementation_type implementation_type;

            static boost::asio::io_service::id id;

            explicit file_monitor_service ( boost::asio::io_service& io_service ) :
                boost::asio::io_service::service( io_service ),
                service_impl_( io_service )
            {
            }

            void construct( implementation_type& impl )
            {
                service_impl_.construct( impl );
            }

            void destroy( implementation_type& impl )
            {
                service_impl_.destroy( impl );
            }

            void add_file ( implementation_type& impl, const std::string& file, boost::system::error_code& ec )
            {
                service_impl_.add_file( impl, file, ec );
            }

            template <typename MonHandler>
                void async_monitor ( implementation_type& impl, boost::system::error_code& ec, BOOST_ASIO_MOVE_ARG( MonHandler ) handler )
                {
                    service_impl_.async_monitor( impl, ec, BOOST_ASIO_MOVE_CAST(MonHandler)(handler) );
                }

        private:
            void shutdown_service()
            {
                service_impl_.shutdown_service();
            }

        private:
            service_impl_type service_impl_;
    };

    boost::asio::io_service::id file_monitor_service::id;
}
