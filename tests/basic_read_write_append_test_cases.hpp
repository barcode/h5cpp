/*
 * The test cases are separated into this file to enable reuse in
 * several test cases without the need to create one cpp testing everything
 * 
 * The including wile has to define:
 * * constexpr std::size_t chunk_sz
 * * constexpr std::size_t data_sz
 * * constexpr std::size_t in_chunk_offset
 * * typedef ::testing::Types<...> BasicRdWrApTestTypes;
 * * std::vector<T> generate(std::size_t seed)
 */

template <typename T> class BasicRdWrApTest : public AbstractTest<T> {};
// instantiate for listed types
TYPED_TEST_CASE(BasicRdWrApTest, BasicRdWrApTestTypes);

template <class T>
struct binary_printer
{
    binary_printer(const T& t) : ptr{(const std::uint8_t*) &t} {}
    const std::uint8_t* ptr;
};
template<class T>
std::ostream& operator<<(std::ostream& out, binary_printer<T> p)
{
    for(std::size_t i = 0; i < sizeof(T); ++i)
    {
        out << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(p.ptr[i]) << std::dec << " ";
    }
    return out;
}

template<class T, class It>
void binaryCompare(It lbeg, It lend, It rbeg, It rend) {
	const auto lsize = std::distance(lbeg, lend);
	const auto rsize = std::distance(rbeg, rend);
	EXPECT_GE(lsize, 0);
	EXPECT_GE(rsize, 0);
	EXPECT_EQ(lsize, rsize);
    
	for(std::size_t idx = 0;lbeg != lend && rbeg != rend; ++lbeg, ++rbeg, ++idx)
	{
            EXPECT_TRUE(binaryEqual(*lbeg, *rbeg))
			<< " element " << idx + 1 << " / " << lsize
			<< "\nlhs = " << *lbeg
            << "\nrhs = " << *rbeg
            << "\nbinary lhs = " << binary_printer(*lbeg)
            << "\nbinary rhs = " << binary_printer(*rbeg);
	}
}

template<class T>
void binaryCompare(const std::vector<T>& l, const std::vector<T>& r) {
	binaryCompare<T>(l.begin(), l.end(), r.begin(), r.end());
}

TYPED_TEST(BasicRdWrApTest, CreateFixedSize) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(fd, path, h5::max_dims{0});
}

TYPED_TEST(BasicRdWrApTest, CreateUnlimitedSize) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(fd, path, h5::max_dims{H5S_UNLIMITED});
}

TYPED_TEST(BasicRdWrApTest, CreateWrite) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(fd, path, h5::max_dims{data_sz});
	{
		const auto data = generate<TypeParam>(0);
		h5::write(ds, data);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data, read);
	}
	{
		const auto data = generate<TypeParam>(1);
		h5::write(ds, data);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data, read);
	}
}

TYPED_TEST(BasicRdWrApTest, WriteDirectly) {
	const auto fd = this->fd;
	const auto path = this->name;
	{
		const auto data = generate<TypeParam>(0);
		h5::write(fd, path, data);
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare(data, read);
	}
	{
		const auto data = generate<TypeParam>(1);
		h5::write(fd, path, data);
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare(data, read);
	}
}

TYPED_TEST(BasicRdWrApTest, WriteDirectlyUnlimited) {
	const auto fd = this->fd;
	const auto path = this->name;
	{
		const auto data = generate<TypeParam>(0);
		h5::write(fd, path, data, h5::max_dims{H5S_UNLIMITED});
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare(data, read);
	}
	{
		const auto data = generate<TypeParam>(1);
		h5::write(fd, path, data, h5::max_dims{H5S_UNLIMITED});
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare(data, read);
	}
}

TYPED_TEST(BasicRdWrApTest, CreateAppendElements) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(
		fd, path, h5::max_dims{H5S_UNLIMITED},
		h5::chunk{chunk_sz}
	);
	h5::pt_t pt{ds};
	const auto data0 = generate<TypeParam>(0);
	const auto data1 = generate<TypeParam>(1);
	{
		for(const auto elem : data0)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data0, read);
	}
	{
		for(const auto elem : data1)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data1.begin(), data1.end(), read.begin() + data0.size(), read.end());
	}
}

/*TYPED_TEST(BasicRdWrApTest, CreateAppendCompressed) {
	const auto fd = this->fd;
	const auto path = this->name;
	TypeParam filling;
	h5::ds_t ds = h5::create<TypeParam>(
		fd, path, h5::max_dims{H5S_UNLIMITED},
		h5::chunk{chunk_sz} | h5::gzip{9} | h5::fill_value<TypeParam>(filling));
	h5::pt_t pt{ds};
	const auto data0 = generate<TypeParam>(0);
	const auto data1 = generate<TypeParam>(1);
	{
		for(const auto elem : data0)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data0, read);
	}
	{
		for(const auto elem : data1)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data1.begin(), data1.end(), read.begin() + data0.size(), read.end());
	}
}*/

TYPED_TEST(BasicRdWrApTest, CreateWriteAppendElements) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(
		fd, path, h5::current_dims{data_sz},
		h5::max_dims{H5S_UNLIMITED},
		h5::chunk{chunk_sz} );
	h5::pt_t pt{ds};
	const auto data0 = generate<TypeParam>(0);
	const auto data1 = generate<TypeParam>(1);
	{
		h5::write(fd, path, data0);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data0, read);
	}
	{
		for(const auto elem : data1)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data1.begin(), data1.end(), read.begin() + data0.size(), read.end());
	}
}

TYPED_TEST(BasicRdWrApTest, WriteDirectlyAppendElements) {
	const auto fd = this->fd;
	const auto path = this->name;
	const auto data0 = generate<TypeParam>(0);
	const auto data1 = generate<TypeParam>(1);
	{
		h5::write(fd, path, data0, h5::max_dims{H5S_UNLIMITED}, h5::chunk{chunk_sz} );
		const auto read = h5::read<std::vector<TypeParam>>(fd, path);
		binaryCompare(data0, read);
	}
	{
		h5::ds_t ds = h5::open(fd, path);
		h5::pt_t pt{ds};
		for(const auto elem : data1)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data1.begin(), data1.end(), read.begin() + data0.size(), read.end());
	}
}

TYPED_TEST(BasicRdWrApTest, CreateAppendWriteElements) {
	const auto fd = this->fd;
	const auto path = this->name;
	h5::ds_t ds = h5::create<TypeParam>(
		fd, path, h5::max_dims{H5S_UNLIMITED},
		h5::chunk{chunk_sz}
	);
	h5::pt_t pt{ds};
	const auto data0 = generate<TypeParam>(0);
	const auto data1 = generate<TypeParam>(1);
	{
		for(const auto elem : data0)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare(data0, read);
	}
	{
		for(const auto elem : data1)
			h5::append(pt, elem);
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data1.begin(), data1.end(), read.begin() + data0.size(), read.end());
	}
	{
		h5::write(fd, path, data0, h5::offset{data0.size()});
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin(), read.begin() + data0.size());
		binaryCompare<TypeParam>(data0.begin(), data0.end(), read.begin() + data0.size(), read.end());
	}
	{
		h5::write(fd, path, data0, h5::offset{in_chunk_offset});
		const auto read = h5::read<std::vector<TypeParam>>(ds);
		binaryCompare<TypeParam>(
			data0.begin(), data0.begin() + in_chunk_offset,
			read.begin(), read.begin() + in_chunk_offset);
		binaryCompare<TypeParam>(
			data0.begin(), data0.end(),
			read.begin()  + in_chunk_offset, read.begin()  + in_chunk_offset + data0.size());
		binaryCompare<TypeParam>(
			data0.end() - data_sz + in_chunk_offset, data0.end(),
			read.end()  - data_sz + in_chunk_offset, read.end());
	}
}

/*----------- BEGIN TEST RUNNER ---------------*/
H5CPP_TEST_RUNNER( int argc, char**  argv );
/*----------------- END -----------------------*/

