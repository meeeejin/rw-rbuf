# RW and R-Buf

## RW

**RW** is **an intuitive and explicit storage command to address read stalls**. Upon read stall, both read and write can be requested to the flash storage in a single call. And the read can be processed as soon as the dirty page is copied to the storage buffer cache using DMA, without waiting for the NAND write to finish. By replacing two I/O calls for write and read upon read stalls with one call, RW can simplify the structure of the buffer manager in DBMS and reduce the number of I/O interrupts and consequently the number of context switches.

### Structure

The major modifications to implement RW were made in the directories below:

- [`storage/innobase/buf`](storage/innobase/buf): The database buffer implementation for InnoDB
- [`storage/innobase/fil`](storage/innobase/fil): File I/O operations for InnoDB
- [`storage/innobase/include`](storage/innobase/include): The collection of header files for InnoDB

RW consists of the following components:

- Basic modules for the buffer manager to utilize the RW command (e.g., buffer initialization, buffer allocation, buffer replacement, etc.)
  - [`buf0buf.cc`](storage/innobase/buf/buf0buf.cc)
- The module for proceeding the transaction immediately using the read data buffer without a separate free buffer acquisition procedure
  - [`buf0rea.cc`](storage/innobase/buf/buf0rea.cc)
  - [`buf0lru.cc`](storage/innobase/buf/buf0lru.cc)
- The module for sending the RW NVM command set to storage
  - [`rw0rw.cc`](storage/innobase/rw/rw0rw.cc), [`rw0rw.h`](storage/innobase/include/rw0rw.h)
  - [`buf0flu.cc`](storage/innobase/buf/buf0flu.cc)
  - [`fil0fil.cc`](storage/innobase/fil/fil0fil.cc)
  - [`os0file.cc`](storage/innobase/os/os0file.cc)

You can check the modified code by searching for the `#ifdef RW_CMD` keyword in the file. For example:

> `storage/innobase/fil/fil0fil.cc`

```cpp
#ifdef RW_CMD
...
fil_mutex_enter_and_prepare_for_rw_io(
/*===============================*/
  ulint space_id, /*!< in: space id to write */
  ulint space_id_to_read /*!< in: space id to read */
)
{
  fil_space_t*  space;
    ...
#endif /* RW_CMD */
```

## R-Buf

