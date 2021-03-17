
//////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2017 Vicon Motion Systems Ltd
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <memory>

class VCGStreamPostalService
{
public:
  void Post( const std::function< void() >& i_rFunction ) const
  {
    m_pService->post( i_rFunction );
  }

  bool StartService()
  {
    boost::mutex::scoped_lock Lock( m_Mutex );
    if( !m_pService )
    {
      m_pService = std::make_shared< boost::asio::io_service >();
    }

    if( !m_pWork )
    {
      m_pWork = std::make_shared< boost::asio::io_service::work >( *m_pService );
      m_Thread = boost::thread( std::bind( &VCGStreamPostalService::ThreadFunction, this ) );
    }

    return true;
  }

  bool StopService()
  {
    boost::mutex::scoped_lock Lock( m_Mutex );
    if( m_pWork )
    {
      m_pWork.reset();
      m_Thread.join();
      m_pService->reset();
      return true;
    }
    return false;
  }

  void ThreadFunction()
  {
    m_pService->run();
  }

private:
  mutable boost::mutex m_Mutex;
  std::shared_ptr< boost::asio::io_service > m_pService;
  std::shared_ptr< boost::asio::io_service::work > m_pWork;
  boost::thread m_Thread;
};
