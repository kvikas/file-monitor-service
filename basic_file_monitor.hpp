#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/error.hpp>

namespace services
{
    template <typename Service>
        class basic_file_monitor :
            public boost::asio::basic_io_object< Service >
        {
            public:

                explicit basic_file_monitor ( boost::asio::io_service& io_service ) :
                    boost::asio::basic_io_object< Service >( io_service )
                {
                }

                void add_file ( const std::string& filename )
                {
                    boost::system::error_code ec;
                    this->service.add_file ( this->implementation, filename, ec );
                    boost::asio::detail::throw_error( ec, "add_file" );
                }

                template <typename MonHandler>
                    void async_monitor( BOOST_ASIO_MOVE_ARG( MonHandler ) handler )
                    {
                        boost::system::error_code ec;
                        this->service.async_monitor ( this->implementation, ec, BOOST_ASIO_MOVE_CAST(MonHandler)(handler) );
                        boost::asio::detail::throw_error( ec, "async_monitor" );
                    }
        };
}
