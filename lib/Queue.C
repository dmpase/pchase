/*******************************************************************************
 * Copyright (c) 2011, Douglas M. Pase                                         *
 * All rights reserved.                                                        *
 * Redistribution and use in source and binary forms, with or without          *
 * modification, are permitted provided that the following conditions          *
 * are met:                                                                    *
 * o       Redistributions of source code must retain the above copyright      *
 *         notice, this list of conditions and the following disclaimer.       *
 * o       Redistributions in binary form must reproduce the above copyright   *
 *         notice, this list of conditions and the following disclaimer in     *
 *         the documentation and/or other materials provided with the          *
 *         distribution.                                                       *
 * o       Neither the name of the copyright holder nor the names of its       *
 *         contributors may be used to endorse or promote products derived     *
 *         from this software without specific prior written permission.       *
 *                                                                             *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   *
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF      *
 * THE POSSIBILITY OF SUCH DAMAGE.                                             *
 *******************************************************************************/


/******************************************************************************
 *                                                                            *
 * Queue                                                                      *
 *                                                                            *
 * Author:  Douglas M. Pase                                                   *
 *                                                                            *
 * Date:    April 15, 2007                                                    *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>

#include "Queue.h"

Queue::Queue() : _next(NULL), _count(0), _object(NULL), _lock(new Lock())
{
}

Queue::Queue(Lock* lock) : _next(NULL), _count(0), _object(NULL), _lock(lock)
{
}

Queue::~Queue()
{
    if (_lock != NULL) {
        delete _lock;
        _lock = NULL;
    }
}

void 
Queue::push(void* obj)
{
    if (this->_lock != NULL) {
        this->_lock->lock();

        Queue* temp   = new Queue(this->_lock);
        temp->_object = obj;
        this->_count += 1;

        if (this->_next == NULL) {
                                // nothing in Queue
            this->_next = temp;
            temp->_next = temp;
        } else {
                                // one or more items in Queue
            temp->_next = this->_next->_next;
            this->_next->_next = temp;
            this->_next = temp;
        }

        this->_lock->unlock();
    }
}

void*
Queue::pull(void)
{
    void* result = NULL;

    if (this->_lock != NULL) {
        this->_lock->lock();

        if (this->_next == NULL) {
            ;			// nothing in Queue
        } else if (this->_next->_next == this->_next) {
                                // one item in Queue
            Queue* tmp = this->_next;
            result = this->_next->_object;
            this->_next = NULL;
            this->_count -= 1;
            tmp->_lock = NULL;
            delete tmp;
        } else {
			    // two or more items in Queue
            Queue* tmp = this->_next->_next;
            result = this->_next->_next->_object;
            this->_next->_next = this->_next->_next->_next;
            this->_count -= 1;
            tmp->_lock = NULL;
            delete tmp;
        }

        this->_lock->unlock();
    }

    return result;
}

void*
Queue::head(void)
{
    void* result = NULL;

    if (this->_lock != NULL) {
        this->_lock->lock();

        if (this->_next == NULL) {
            ;			// nothing in Queue
        } else {
				// one or more items in Queue
            result = this->_next->_next->_object;
        }

        this->_lock->unlock();
    }

    return result;
}

void*
Queue::tail(void)
{
    void* result = NULL;

    if (this->_lock != NULL) {
        this->_lock->lock();

        if (this->_next == NULL) {
            ;			// nothing in Queue
        } else {
				// one or more items in Queue
            result = this->_next->_object;
        }

        this->_lock->unlock();
    }

    return result;
}
