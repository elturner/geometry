#ifndef THREADED_BUFFER_H
#define THREADED_BUFFER_H

/**
 * @file threaded_buffer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the threaded_buffer_t class, which is a
 * wrapper for file output streams (either ascii or binrary),
 * which will buffer the data then write it out on a seperate
 * thread in an efficient manner.
 */

#include <fstream>
#include <stdlib.h>
#include <windows.h>

/**
 * The threaded buffer class for file output streams.
 */
class threaded_buffer_t
{
	/* parameters */
	private:

		/* the output stream to disk */
		std::ofstream outfile;

		/* the following specifies a buffer that is used to store
		 * data that will eventually be written to disk.  This 
		 * buffer is processed as a ring buffer. */
		char* buf;

		/* the buffer is partitioned into a list of chunks, so that
		 * the total size of the buffer is 
		 * num_chunks * chunk_size */
		size_t chunk_size; /* units: bytes */
		size_t num_chunks; /* units: chunks */
		size_t buf_size;   /* units: bytes */

		/* the write position denotes the next unused byte in 
		 * the buffer */
		size_t write_pos; /* units: bytes */

		/* the read position denotes the next byte that has not 
		 * been written to disk (and so only exists in this 
		 * buffer).  This value should only ever be accessed from 
		 * the reader thread while both threads are active. */
		size_t read_pos; /* units: bytes */

		/* these positions are observed by both the reader and 
		 * writer threads, so they need their own mutexes */
		SRWLOCK write_pos_lock;
		SRWLOCK read_pos_lock;

		/* each chunk in the buffer has a mutex, so that it cannot 
		 * be read from and written to at the same time. 
		 * Array size: num_chunks */
		SRWLOCK* chunk_locks;
		
		/* the reader portion of this class (which copies data 
		 * from buffer to disk) runs on a seperate thread, which 
		 * is stored here */
		HANDLE reader_thread;

		/* the following boolean indicates to the reader thread
		 * whether or not to continue processing.  Only the main 
		 * thread should ever change this boolean.  When changing 
		 * from false -> true, the next call should be to create 
		 * the reader thread.  Upon changing from true -> false, 
		 * the next call should be to join with the
		 * reader thread. */
		 bool reader_thread_continue_processing;

		 /* this variable specifies the sleep period for the 
		  * reader thread, in units of milliseconds */
		 unsigned int reader_thread_sleep_period;

	/* functions */
	public:

		/* constructors */

		/**
		 * Default constructor
		 */
		threaded_buffer_t();

		/**
		 * Frees all resources and threads associated with object.
		 *
		 * Will close any open file streams, free all memory, 
		 * and join all threads that are associated with this class.
		 */
		~threaded_buffer_t();

		/* file operations */

		/**
		 * Will open a filestream to the specified file location
		 *
		 * Given a filename, will open a filestream that will write
		 * to this location.  If a filestream is already open, 
		 * then will close that and open this one, instead.
		 * 
		 * @param filename The name of the file to write to.
		 * @param cs       The chunk size to use
		 * @param nc       The number of chunks to use
		 * @param sp       The reader thread sleep period to use
		 */
		void open(const char* filename, size_t cs, size_t nc, 
		          unsigned int sp);

		/**
		 * Specifies if a file stream is open for this class.
		 *
		 * Will return true if this stream is active, and writing to
		 * a file on disk.  Otherwise, will return false.
		 * 
		 * @return Returns true iff output stream open
		 */
		bool is_open() const;

		/**
		 * Will write the specified data string to the buffer.
		 *
		 * The n bytes of data that begin at location s will be
		 * copied to the buffer, and will be queued to be written
		 * to disk.
		 * 
		 * @param s The data pointer to be copied into buffer
		 * @param n The size of the data to be copied
		 */
		void write(const char* s, size_t n);

		/**
		 * Flushes the buffer to disk.
		 *
		 * Any data stored in the buffer that has not yet been 
		 * written to disk will be forced to disk.  After this 
		 * call, the full buffer will be free.
		 *
		 * NOTE: This operation will be SLOW
		 */
		void flush();

		/**
		 * Closes the file stream, if open.
		 *
		 * Will flush and close the filestream, if open.  
		 * Has no effect if open() has not yet been called.
		 */
		void close();

	/* helper functions */
	public:

		/**
		 * Returns the chunk size
		 *
		 * Returns the size of a single chunk in the buffer of 
		 * this object
		 * 
		 * @return Returns the value of chunk_size
		 */
		inline size_t get_chunk_size()
		{
			return this->chunk_size;
		};

		/**
		 * Returns the value of reader_thread_continue_processing
		 *
		 * This parameter is set to true if the reader thread 
		 * should continue looping.  If this value is false, the
		 * reader thread will flush the buffer and dispose.
		 * 
		 * @return Returns reader_thread_continue_processing
		 */
		inline bool get_reader_thread_continue_processing()
		{
			return this->reader_thread_continue_processing;
		};

		/**
		 * Retrieves the sleep period for the reader thread
		 *
		 * The reader thread will sleep for this amount of time (in
		 * milliseconds) while waiting for the buffer to fill.
		 * 
		 * @return Returns the sleep cycle length for the reader
		 */
		inline unsigned int get_reader_thread_sleep_period()
		{
			return this->reader_thread_sleep_period;
		};

		/**
		 * Computes chunk that contains the specified byte location
		 * 
		 * @param  i The byte index to analyze
		 * 
		 * @return   The index of the chunk that contains the input
		 */
		inline size_t chunk_index_of(size_t i)
		{
			return (i / this->chunk_size);
		};	

		/**
		 * Computes remaining bytes in current chunk.
		 * 
		 * Computes the number of bytes that fall between the 
		 * specified position, and the end of the its containing 
		 * chunk.  If argument i denotes the first byte of a chunk,
		 * then the value returned will be the total number of 
		 * bytes in the chunk.  If i denotes the last byte in a 
		 * chunk, the value returned will be 1.
		 *
		 * @param i The byte index to analyze
		 *
		 * @return Returns remaining bytes in current chunk
		 */
		inline size_t bytes_to_chunk_end(size_t i)
		{
			return (this->chunk_size - (i % this->chunk_size));
		};

		/**
		 * Returns a snapshot of the write position.
		 *
		 * Since the write pos is accessed by both threads,
		 * this function will retrieve a value of the write pos
		 * while under mutex.  However, since after returning
		 * the mutex is no longer locked, this value is instantly
		 * out-of-date.  It can be used as a minimum bound
		 * for the write pos (since the write pos only increases).
		 *
		 * @return Returns a snahpshot of the write position
		 */
		inline size_t get_write_pos()
		{
			size_t w;

			/* acquire a snapshot of the write position */
			AcquireSRWLockShared(&(this->write_pos_lock));
			w = this->write_pos;
			ReleaseSRWLockShared(&(this->write_pos_lock));

			/* return snapshot */
			return w;
		};

		/**
		 * Returns a snapshot of the read position.
		 *
		 * Since the read pos is accessed by both threads,
		 * this function will retrieve a value of the read pos
		 * while under mutex.  However, since after returning
		 * the mutex is no longer locked, this value is instantly
		 * out-of-date.  It can be used as a minimum bound
		 * for the read pos (since the read pos only increases).
		 *
		 * @return Returns a snahpshot of the read position
		 */
		inline size_t get_read_pos()
		{
			size_t r;

			/* acquire a snapshot of the read position */
			AcquireSRWLockShared(&(this->read_pos_lock));
			r = this->read_pos;
			ReleaseSRWLockShared(&(this->read_pos_lock));

			/* return snapshot */
			return r;
		};

		/**
		 * Increments the write position (modulo buffer size)
		 *
		 * This should only be called from the main thread of
		 * the class, not from the read thread.
		 * 
		 * @param n Amount to increment write position
		 */
		inline void increment_write_pos(size_t n)
		{
			AcquireSRWLockExclusive(&(this->write_pos_lock));
			this->write_pos += n;
			this->write_pos %= this->buf_size;
			ReleaseSRWLockExclusive(&(this->write_pos_lock));
		};

		/**
		 * Increments the read position (modulo buffer size)
		 *
		 * This should only be called from the read thread,
		 * not from the main thread of the class.
		 * 
		 * @param n Amount to increment read position
		 */
		inline void increment_read_pos(size_t n)
		{
			AcquireSRWLockExclusive(&(this->read_pos_lock));
			this->read_pos += n;
			this->read_pos %= this->buf_size;
			ReleaseSRWLockExclusive(&(this->read_pos_lock));
		};

		/**
		 * Will write next portion of buffer to file
		 *
		 * Helper function that should only be called from within
		 * the reader loop.  This will write the next 
		 * amount_to_stream bytes, starting from the read position,
		 * to the output file stream.  It will use the specified 
		 * buffer to help this copy of data.
		 * 
		 * @param chunk_index      The chunk index containing data
		 * @param amount_to_stream Amount to stream (<= chunk size)
		 * @param local_buf        Buffer to use as temp storage
		 */
		inline void chunk_portion_to_stream(size_t chunk_index,
		                                    size_t amount_to_stream,
		                                    char*  local_buf)
		{
			/* copy data from buffer */
			AcquireSRWLockShared(
			       &(this->chunk_locks[chunk_index]));
			memcpy(local_buf, this->buf + this->read_pos,
			       amount_to_stream);
			ReleaseSRWLockShared(
			       &(this->chunk_locks[chunk_index]));

			/* write data to file */
			this->outfile.write(local_buf, amount_to_stream);

			/* update read position to after these data */
			this->increment_read_pos(amount_to_stream);
		};

	/* debugging functions */
	public:

		/**
		 * Will print the contents of the buffer to screen.
		 */
		void print_to_screen();
};

#endif
