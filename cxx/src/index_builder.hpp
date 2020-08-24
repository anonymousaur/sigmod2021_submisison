#include "index_builder.h"

#include "utils.h"

template <size_t D>
std::unique_ptr<PrimaryIndexer<D>> IndexBuilder<D>::Build(std::string spec) {
    std::ifstream f(spec);
    assert (f.is_open());
    auto base_ptr = Dispatch(f, true);
    AssertWithMessage(base_ptr->Type() == IndexerType::Primary, "Root index must be a primary index");
    std::unique_ptr<PrimaryIndexer<D>> indexer(dynamic_cast<PrimaryIndexer<D>*>(base_ptr.release()));
    return indexer;
}

template <size_t D>
std::unique_ptr<Indexer<D>> IndexBuilder<D>::Dispatch(std::ifstream& spec, bool root) {
    std::string next_index;
    spec >> next_index;
    if (next_index == "DummyIndex") {
        return BuildDummyIndex(spec);
    } else if (next_index == "CompositeIndex") { 
        return BuildCompositeIndex(spec);
    } else if (next_index == "CombinedCorrelationIndex") {
        assert (!root);
        return BuildCombinedCorrelationIndex(spec);
    } else if (next_index == "MappedCorrelationIndex") {
        assert (!root);
        return BuildMappedCorrelationIndex(spec);
    } else if (next_index == "SecondaryBTreeIndex") {
        assert (!root);
        return BuildSecondaryBTreeIndex(spec);
    } else if (next_index == "BinarySearchIndex") {
        return BuildBinarySearchIndex(spec);
    } else if (next_index == "PrimaryBTreeIndex") {
        return BuildPrimaryBTreeIndex(spec);
    } else if (next_index == "OctreeIndex") {
        return BuildOctreeIndex(spec);
    } else if (next_index == "}") {
        // Case to deal with variable length indexes
        return nullptr;
    }
    AssertWithMessage(false, "Index type " + next_index + " not recognized");
    return nullptr;    
}

template <size_t D>
std::unique_ptr<DummyIndex<D>> IndexBuilder<D>::BuildDummyIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    spec >> parens.first  >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for JustSortIndex");
    std::cout << "Building DummyIndex" << std::endl;
    return std::make_unique<DummyIndex<D>>();
}

template <size_t D>
std::unique_ptr<JustSortIndex<D>> IndexBuilder<D>::BuildJustSortIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    size_t dim;
    spec >> parens.first >> dim >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for JustSortIndex");
    std::cout << "Building JustSortIndex on dim " << dim << std::endl;
    return std::make_unique<JustSortIndex<D>>(dim);
}

template <size_t D>
std::unique_ptr<SecondaryBTreeIndex<D>> IndexBuilder<D>::BuildSecondaryBTreeIndex(std::ifstream& spec) {
    std::string paren, optional_list, paren2;
    size_t dim;
    spec >> paren >> dim >> optional_list;
    if (optional_list == "}") {
        AssertWithMessage(paren == "{", "Incorrect spec for SecondaryBTreeIndex");
        return std::make_unique<SecondaryBTreeIndex<D>>(dim);
        std::cout << "Building SecondaryBTreeIndex with dim " << dim << std::endl;
    }
    spec >> paren2;
    AssertWithMessage(paren == "{" && paren2 == "}", "Incorrect spec for SecondaryBTreeIndex");
    IndexList outlier_list = load_binary_file<size_t>(optional_list);
    std::cout << "Building SecondaryBTreeIndex with dim " << dim << " and outlier list of size " << outlier_list.size() << std::endl;
    return std::make_unique<SecondaryBTreeIndex<D>>(dim, outlier_list);
}

template <size_t D>
std::unique_ptr<BinarySearchIndex<D>> IndexBuilder<D>::BuildBinarySearchIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    size_t dim;
    spec >> parens.first >> dim >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for BinarySearchIndex");
    std::cout << "Building BinarySearchIndex on dim " << dim << std::endl;
    return std::make_unique<BinarySearchIndex<D>>(dim);
}

template <size_t D>
std::unique_ptr<PrimaryBTreeIndex<D>> IndexBuilder<D>::BuildPrimaryBTreeIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    size_t dim, page_size;
    spec >> parens.first >> dim >> page_size >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for PrimaryBTreeIndex");
    std::cout << "Building PrimaryBTreeIndex with dim " << dim << " and page size " << page_size << std::endl;
    return std::make_unique<PrimaryBTreeIndex<D>>(dim, page_size);
}

template <size_t D>
std::unique_ptr<MappedCorrelationIndex<D>> IndexBuilder<D>::BuildMappedCorrelationIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    std::string mapping_file, target_bucket_file;
    spec >> parens.first >> mapping_file >> target_bucket_file >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for MappedCorrelationIndex");
    std::cout << "Building MappedCorrelationIndex" << std::endl;
    return std::make_unique<MappedCorrelationIndex<D>>(mapping_file, target_bucket_file);
}

template <size_t D>
std::unique_ptr<CombinedCorrelationIndex<D>> IndexBuilder<D>::BuildCombinedCorrelationIndex(std::ifstream& spec) {
    std::pair<std::string, std::string> parens;
    spec >> parens.first;
    auto map_index = Dispatch(spec);
    auto sec_index = Dispatch(spec);
    spec >> parens.second;
    AssertWithMessage(parens.first == "{" && parens.second == "}", "Incorrect spec for CombinedCorrelationIndex");
    auto comb_index = std::make_unique<CombinedCorrelationIndex<D>>();
    comb_index->SetMappedIndex(std::unique_ptr<MappedCorrelationIndex<D>>(
                dynamic_cast<MappedCorrelationIndex<D>*>(map_index.release())));
    std::cout << "Adding MappedCorrelationIndex to CombinedCorrelationIndex" << std::endl;
    comb_index->SetOutlierIndex(std::unique_ptr<SecondaryIndexer<D>>(
                dynamic_cast<SecondaryIndexer<D>*>(sec_index.release())));
    std::cout << "Adding SecondaryIndex to CombinedCorrelationIndex" << std::endl;
    std::cout << "Building CombinedCorrelationIndex" << std::endl;
    return comb_index;
}

template <size_t D>
std::unique_ptr<CompositeIndex<D>> IndexBuilder<D>::BuildCompositeIndex(std::ifstream& spec) {
    std::string paren1;
    spec >> paren1;
    AssertWithMessage(paren1 == "{", "Incorrect spec for CompositeIndex");
    auto composite_index = std::make_unique<CompositeIndex<D>>(0);
    while (true) {
        auto next_index = Dispatch(spec);
        if (!next_index) {
            break;
        }
        switch (next_index->Type()) {
            case Primary:
                composite_index->SetPrimaryIndex(std::unique_ptr<PrimaryIndexer<D>>(
                            static_cast<PrimaryIndexer<D>*>(next_index.release())));
                std::cout << "Setting primary index for CompositeIndex" << std::endl;
                break;
            case Secondary:
                composite_index->AddSecondaryIndex(std::unique_ptr<SecondaryIndexer<D>>(
                            static_cast<SecondaryIndexer<D>*>(next_index.release())));
                std::cout << "Adding secondary index to CompositeIndex" << std::endl;
                break;
            case Correlation:
                composite_index->AddCorrelationIndex(std::unique_ptr<CorrelationIndexer<D>>(
                            static_cast<CorrelationIndexer<D>*>(next_index.release())));
                std::cout << "Adding correlation index to CompositeIndex" << std::endl;
                break;
        }
    }
    std::cout << "Building CompositeIndex" << std::endl;
    return composite_index;
} 

template <size_t D>
std::unique_ptr<OctreeIndex<D>> IndexBuilder<D>::BuildOctreeIndex(std::ifstream& spec) {
    std::string token;
    spec >> token;
    AssertWithMessage(token == "{", "Incorrect spec for OctreeIndex");
    std::vector<std::string> params;
    while (spec >> token) {
        if (token == "}") {
            break;
        }
        params.push_back(token);
    }
    AssertWithMessage(params.size() > 1, "Octree requires page_size and at least one indexed dimension");
    size_t page_size = std::stoi(params[0]);
    std::vector<size_t> indexed_dims;
    for (size_t i = 1; i < params.size(); i++) {
        indexed_dims.push_back(std::stoi(params[i]));
    }
    return std::make_unique<OctreeIndex<D>>(indexed_dims, page_size);
}
        
