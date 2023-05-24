
struct kq {
  u64 nread;
  u64 nwritten;
  u8 log_size;
  u8 data[64];
};

static inline u64 kq_mask(struct kq *reg)
{
  return (1 << reg->log_size) - 1;
}

static inline u64 kq_size(struct kq *reg)
{
  return (reg->nread - reg->nwritten) & kq_mask(reg);
}

static inline u64 kq_free_space(struct kq *reg)
{
  return kq_mask(reg) - kq_size(reg);
}

static ssize_t kq_write(struct kq *reg, const u8 *buf, size_t n)
{
  u64 start = reg->nwritten & kq_mask(reg);
  // if start == end, queue is empty
  u64 end = (reg->nread - 1) & kq_mask(reg);
  u64 free_space = (end - start) & kq_mask(reg);
  u64 buf_size = kq_mask(reg) + 1;

  pr_trace("kwrite end=%lu start=%lu n=%zu mask=%lx %lu\n", end, start, n, kq_mask(reg), free_space);
  if (free_space < n)
    return -ENOSPC;

  if (start + n <= buf_size) {
    memcpy(&reg->data[start], buf, n);
  } else {
    u64 to_end = buf_size - start;
    u64 from_start = n - to_end;
    WARN_ON(start < end);
    pr_trace("to_end=%lu n=%lu size=%lu\n", to_end, n, kq_mask(reg)+1);
    WARN_ON(to_end >= n);
    memcpy(&reg->data[start], buf, to_end);
    memcpy(&reg->data[0], buf+to_end, from_start);
  }
  reg->nwritten += n;
  return n;
}

static ssize_t kq_read(struct kq *reg, u8 *buf, size_t n)
{
  u64 end = reg->nwritten & kq_mask(reg);
  u64 start = reg->nread & kq_mask(reg);
  // queue is full when end == start-1. We waste a single byte
  u64 q_size = (end - start) & kq_mask(reg);
  u64 buf_size = kq_mask(reg) + 1;
  n = min_t(size_t, n, q_size);
  pr_trace("kq_read n=%lu q_size=%lu start=%lu end=%lu\n", n, q_size, start, end);

  if (start + n <= buf_size) {
    memcpy(buf, &reg->data[start], n);
  } else {
    u64 to_end = buf_size - start;
    u64 from_start = n - to_end;
    WARN_ON(start < end);
    WARN_ON(to_end >= n);
    memcpy(buf, &reg->data[start], to_end);
    memcpy(buf+to_end, &reg->data[0], from_start);
  }
  reg->nread += n;
  return n;
}

