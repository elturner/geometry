#include "threaded_buffer.h"
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <util/error_codes.h>

/**
 * @file threaded_buffer.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the threaded_buffer_t class, which is a
 * wrapper for file output streams (either ascii or binrary),
 * which will buffer the data then write it out on a seperate
 * thread in an efficient manner.
 */

/**
 * Function that denotes the reader thread processing loop.
 * 
 * This function denotes the reader loop, which runs in a separate
 * thread, and progressively moves the contents of the buffer to
 * disk in specified chunks.  It will continue processing as long
 * as the reader_thread_continue_processing paramter is true.
 *
 * @param lpParam A pointer to the calling threaded_buffer_t object
 *
 * @return Returns zero
 */
DWORD WINAPI reader_thread_loop(LPVOID lpParam);

/*** function implementations ***/

threaded_buffer_t::threaded_buffer_t()
{
	/* initialize unopened buffer stream */
	this->buf = NULL;
	this->num_chunks = 0;
	this->chunk_size = 0;
	this->buf_size = 0;
	this->write_pos = 0;
	this->read_pos = 0;
	InitializeSRWLock(&(this->write_pos_lock));
	InitializeSRWLock(&(this->read_pos_lock));
	this->chunk_locks = NULL;
	this->reader_thread = NULL;
	this->reader_thread_continue_processing = false;
	this->reader_thread_sleep_period = 0;
}

threaded_buffer_t::~threaded_buffer_t()
{
	/* close any open streams */
	this->close();

	/* free memory */
	free(this->buf);
	free(this->chunk_locks);
}

void threaded_buffer_t::open(const char* filename, size_t cs,
                             size_t nc, unsigned int sp)
{
	size_t bs;
	unsigned int i;

	/* close open stream if necessary */
	this->close();

	/* force number of chunks to be at least two */
	if(nc <= 1)
		nc = 2;

	/* resize buffer if necessary */
	bs = cs * nc;
	if(!(this->buf) || this->num_chunks * this->chunk_size < bs)
	{
		/* need to reallocate buffer */
		this->buf = (char*) realloc(this->buf, bs);
	}

	/* resize mutex array if necessary */
	if(this->num_chunks != nc)
	{
		/* reallocate */
		this->chunk_locks = (SRWLOCK*) realloc(this->chunk_locks, 
		                                    nc * sizeof(SRWLOCK));

		/* initialize each lock */
		for(i = 0; i < nc; i++)
			InitializeSRWLock(&(this->chunk_locks[i]));
	}

	/* update sizes */
	this->chunk_size = cs;
	this->num_chunks = nc;
	this->buf_size = bs;

	/* reset buffer positions */
	this->write_pos = 0;
	this->read_pos = 0;

	/* open file for writing */
	outfile.open(filename, std::ios::out | std::ios::binary);
	if(!(this->outfile.is_open()))
		return;

	/* start reader thread */
	this->reader_thread_continue_processing = true;
	this->reader_thread_sleep_period = sp;
	this->reader_thread = CreateThread(NULL, /* default security */
	                                   0, /* default stack size */
	                                   reader_thread_loop, /* function */
	                                   (void*) this, /* argument to function */
	                                   0, /* default flags */
	                                   NULL); /* don't need thread id */

	/* verify thread has started */
	if(this->reader_thread == NULL)
		this->close();

	/* set thread priority */
	SetThreadPriority(this->reader_thread, /* handle */
	                  THREAD_PRIORITY_BELOW_NORMAL /* priority of thread */
	                  );
}

bool threaded_buffer_t::is_open() const
{
	return this->outfile.is_open();
}

void threaded_buffer_t::write(const char* s, size_t n)
{
	size_t i, m, r, w, chunk_index;

	/* check arguments and parameters */
	if(s == NULL || this->buf == NULL 
			|| this->num_chunks == 0 || this->chunk_size == 0 
			|| this->buf_size == 0 || this->chunk_locks == NULL)
		return;

	/* write data by chunks */
	i = 0;
	while(i < n)
	{
		/* get the amount to read from given string */
		w = this->get_write_pos();
		m = min(n - i, this->bytes_to_chunk_end(w));

		/* check if this write will overflow the buffer */
		r = this->get_read_pos();
		if(w < r && w + m >= r)
		{
			/* writing this chunk will overflow the buffer,
			 * so drop the remaining data on the floor */
			this->flush();
			continue;
		}

		/* copy this amount into the buffers */
		chunk_index = this->chunk_index_of(w);
		AcquireSRWLockExclusive(&(this->chunk_locks[chunk_index]));
		memcpy(this->buf + w, s + i, m);
		ReleaseSRWLockExclusive(&(this->chunk_locks[chunk_index]));
		
		/* increment the string */
		i += m;
		this->increment_write_pos(m);
	}
}

void threaded_buffer_t::flush()
{
	/* join reader thread (which will copy remaining contents of
	 * buffer to the file) */
	this->reader_thread_continue_processing = false;
	WaitForSingleObject(this->reader_thread, INFINITE);
	this->reader_thread = NULL;

	/* restart the reader thread, to continue using this buffer class */
	this->reader_thread_continue_processing = true;
	this->reader_thread = CreateThread(NULL, /* default security */
	                                   0, /* default stack size */
	                                   reader_thread_loop, /* function */
	                                   (void*) this, /* argument to function */
	                                   0, /* default flags */
	                                   NULL); /* don't need thread id */

	/* set thread priority */
	SetThreadPriority(this->reader_thread, /* handle */
	                  THREAD_PRIORITY_BELOW_NORMAL /* priority of thread */
	                  );
}

void threaded_buffer_t::close()
{
	/* check if thread active */
	if(this->reader_thread != NULL)
	{
		/* join reader thread (which will copy remaining contents of
		 * buffer to the file) */
		this->reader_thread_continue_processing = false;
		WaitForSingleObject(this->reader_thread, INFINITE);
		this->reader_thread = NULL;
	}

	/* close stream if open */
	if(this->outfile.is_open())
		this->outfile.close();
}

/* function that denotes the reader thread processing loop. */
DWORD WINAPI reader_thread_loop(LPVOID lpParam)
{
	size_t to_read, read_pos, write_pos, read_chunk, write_chunk;
	threaded_buffer_t* p;
	char* local_buf;
	bool keep_going;

	/* get pointer to class */
	p = (threaded_buffer_t*) lpParam;

	/* create local buffer for the size of one chunk */
	local_buf = new char[p->get_chunk_size()];

	/* initialize position values arbitrarily */
	write_pos = 1;
	read_pos  = 0;
	keep_going = true;

	/* loop as long as specified */
	while(keep_going || write_pos != read_pos)
	{
		/* get read/write positions */
		keep_going  = p->get_reader_thread_continue_processing();
		read_pos    = p->get_read_pos();
		read_chunk  = p->chunk_index_of(read_pos);
		write_pos   = p->get_write_pos(); /* only valid if flushing */
		write_chunk = p->chunk_index_of(write_pos);

		/* get range of current chunk to read */
		to_read = p->bytes_to_chunk_end(read_pos);

		/* check if there is enough to read to warrent copying */
		if(write_chunk == read_chunk && write_pos >= read_pos)
		{
			/* check if we are trying to flush the entire buffer */
			if(keep_going)
			{
				/* sleep for a bit, then try again */
				Sleep(p->get_reader_thread_sleep_period());
				continue;
			}
			else
			{
				/* we want to flush the entire buffer, so export
				 * this end bit as well (which may not go all the
				 * way to the end of the chunk) 
				 * 
				 * We can do a simple subtraction here, since we
				 * know both read and write positions are in the
				 * same chunk, so it doesn't wrap around the ring */
				 to_read = write_pos - read_pos;
			}
		}

		/* write current data to file.
		 *
		 * First copy the data to a local buffer, so that
		 * this thread isn't hogging the mutex for this chunk
		 * during a system i/o call. */
		 if(to_read > 0)
			p->chunk_portion_to_stream(read_chunk, to_read, local_buf);
	}

	/* success */
	delete[] local_buf;
	return 0;
}

/******** debugging functions ********/

#include <iostream>

using namespace std;

void threaded_buffer_t::print_to_screen()
{
	size_t i;

	/* print contents of buffer */
	cout << "\t[";
	cout.write(this->buf, this->buf_size);
	cout << "]\n";

	/* print read/write position */
	cout << "\t[";
	for(i = 0; i < this->buf_size; i++)
		if(i == this->write_pos && i == this->read_pos)
			cout << "b";
		else if(i == this->write_pos)
			cout << "w";
		else if(i == this->read_pos)
			cout << "r";
		else if(i % this->chunk_size == 0)
			cout << "|";
		else
			cout << ".";
	cout << "]\n\n";
}