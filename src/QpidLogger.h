/*
 * Copyright 2013-2014 Paul Colby
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @brief Declares the QpidLogger class.
 */

#ifndef __QPID_PMDA_QPID_LOGGER_H__
#define __QPID_PMDA_QPID_LOGGER_H__

#include <qpid/log/Logger.h>

/**
 * @brief Qpid logging output sink.
 *
 * This implements the qpid::log::Logger::Output sink interface, to log all Qpid
 * log statements via PCP's __pmNotifyErr function.
 */
class QpidLogger : public qpid::log::Logger::Output {

public:

    virtual void log(const qpid::log::Statement &statement,
                     const std::string &message);

protected:

    static int pmNotifyLevel(const qpid::log::Level level);

};

#endif
