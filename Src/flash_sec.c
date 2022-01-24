#include "flash_sec.h"
#include "flash.h"

int flash_sec_erase(struct flash_sec const* sec)
{
	return flash_erase_sec(sec->no);
}

int flash_sec_write(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	return flash_write(sec->base + off, data, sz);
}

int flash_sec_write_bytes(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	return flash_write_bytes(sec->base + off, data, sz);
}

