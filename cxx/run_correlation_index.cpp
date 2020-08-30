#include <iostream>
#include <algorithm>
#include <chrono>
#include <sysexits.h>
#include <vector>

#include "types.h"
#include "flags.h"
#include "index_builder.h"
#include "row_order_dataset.h"
#include "inmemory_column_order_dataset.h"
#include "linear_model_rewriter.h"
#include "query_engine.h"
#include "visitor.h"
#include "utils.h"
#include "datacube.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Expected arguments: --dataset --workload --visitor "
            << "[--outlier-list] [--mapping-file] [--target-bucket-file] [--page-size] [--composite-gap]"
            << "[--save] [--results_folder]" << std::endl;
        return EX_USAGE;
    }
    auto flags = ParseFlags(argc, argv);

    cout << "Dimension is " << DIM << endl;

    std::string dataset_file = GetRequired(flags, "dataset");
    auto data = load_binary_file< Point<DIM> >(dataset_file);
    std::string workload_file = GetRequired(flags, "workload");
    vector< Query<DIM> > workload = load_query_file<DIM>(workload_file);
    std::cout << "Loaded " << workload.size() << " queries" << std::endl;

    std::cout << "Loaded dataset and workload" << std::endl;

    int num_queries = std::stoi(GetWithDefault(flags, "num_queries", "100000"));
    num_queries = std::min(num_queries, (int)workload.size());

    // Define datacubes
    auto index_creation_start = std::chrono::high_resolution_clock::now();
    
    size_t num_outliers = 0;
    int page_size = std::stoi(GetWithDefault(flags, "page-size", "1"));
    int composite_gap = std::stoi(GetWithDefault(flags, "composite-gap", "1"));

    IndexBuilder<DIM> ix_builder;
    auto indexer = ix_builder.Build(GetRequired(flags, "indexer-spec"));
#ifdef USE_LINEAR
    auto rewriter = std::make_unique<LinearModelRewriter<DIM>>(GetRequired(flags, "rewriter"));
#endif

    std::cout << "Sorting data..." << std::endl;   
    auto before_sort_time = std::chrono::high_resolution_clock::now();
    indexer->Init(data.begin(), data.end());
    auto after_sort_time = std::chrono::high_resolution_clock::now();
    auto tt_sort = std::chrono::duration_cast<std::chrono::nanoseconds>(after_sort_time - before_sort_time).count();
    std::cout << "Time to sort and finalize data: " << tt_sort / 1e9 << "s" << std::endl;

    auto compression_start = std::chrono::high_resolution_clock::now();
    std::vector<Datacube<DIM>> cubes;
    std::cout << "Not using datacubes" << std::endl;
    bool use_datacubes = false;

    //auto dataset = std::make_unique<RowOrderDataset<DIM>>(data);
    auto dataset = std::make_unique<InMemoryColumnOrderDataset<DIM>>(data);
    auto compression_finish = std::chrono::high_resolution_clock::now();
    auto compression_time = std::chrono::duration_cast<std::chrono::nanoseconds>(compression_finish-compression_start).count();
    std::cout << "Compression time: " << compression_time / 1e9 << "s" << std::endl;
    size_t indexer_size_bytes = indexer->Size();
    std::cout << "Indexer sizei (B): " << indexer_size_bytes << std::endl;

#ifdef USE_LINEAR
    QueryEngine<DIM> engine(std::move(dataset), std::move(indexer), std::move(rewriter));
#else
    QueryEngine<DIM> engine(std::move(dataset), std::move(indexer));
#endif
    auto index_creation_finish = std::chrono::high_resolution_clock::now();
    auto index_creation_time = std::chrono::duration_cast<std::chrono::nanoseconds>(index_creation_finish-index_creation_start).count();
    std::cout << "Index creation time: " << index_creation_time / 1e9 << "s" << std::endl;

    // cout << "Index size: " << index.size() << " bytes" << std::endl;

    cout << "Starting queries..." << endl;
    size_t total = 0;
    Scalar aggregate = 0;
    long long points_scanned = 0;
    std::vector<double> query_times;
    query_times.reserve(num_queries);
    std::vector<size_t> results;
    results.reserve(num_queries);
    std::string visitor_type = std::string(GetRequired(flags, "visitor"));
    std::string timeout_str = GetWithDefault(flags, "timeout", "no");
    bool timeout = timeout_str == "yes";
    auto start = std::chrono::high_resolution_clock::now();
    if (visitor_type == std::string("collect")) {
        for (int i = 0; i < num_queries; i++) {
            Query<DIM> q = workload[i];
            auto query_start = std::chrono::high_resolution_clock::now();
            CollectVisitor<DIM> visitor;
            engine.Execute(q, visitor);
            total += visitor.result_set.size();
            results.push_back(visitor.result_set.size());
            auto query_end = std::chrono::high_resolution_clock::now();
            query_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(query_end-query_start).count());
        }
    } else if (visitor_type == std::string("count")) {
        for (int i = 0; i < num_queries; i++) {
            Query<DIM> q = workload[i];
            auto query_start = std::chrono::high_resolution_clock::now();
            CountVisitor<DIM> visitor;
            engine.Execute(q, visitor);
            total += visitor.count;
            results.push_back(visitor.count);
            auto query_end = std::chrono::high_resolution_clock::now();
            query_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(query_end-query_start).count());
        }
    } else if (visitor_type == std::string("index")) {
        long prev_r = 0;
        long prev_l = 0;
        for (int i = 0; i < num_queries; i++) {
            Query<DIM> q = workload[i];
            auto query_start = std::chrono::high_resolution_clock::now();
            IndexVisitor<DIM> visitor;
            engine.Execute(q, visitor);
            total += visitor.indexes.size();
            auto query_end = std::chrono::high_resolution_clock::now();
            results.push_back(visitor.indexes.size());
            long tot_r_scanned = engine.ScannedRangePoints();
            long tot_l_scanned = engine.ScannedListPoints();
            std::cout << "True: " << visitor.indexes.size()
                << ", scanned range = " << tot_r_scanned - prev_r
                << ", scanned list = " << tot_l_scanned - prev_l
                << std::endl;
            prev_r = tot_r_scanned;
            prev_l = tot_l_scanned;
            auto query_t = std::chrono::duration_cast<std::chrono::nanoseconds>(query_end-query_start).count();
            query_times.push_back(query_t);
            std::cout << "Query time (ms): " << query_t/ 1e6 << std::endl;
        }
    } else if (visitor_type == std::string("dummy")) {
        for (int i = 0; i < num_queries; i++) {
            Query<DIM> q = workload[i];
            auto query_start = std::chrono::high_resolution_clock::now();
            DummyVisitor<DIM> visitor;
            engine.Execute(q, visitor);
            auto query_end = std::chrono::high_resolution_clock::now();
            query_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(query_end-query_start).count());
        }
    } else if (visitor_type == std::string("sum")) {
        long prev = 0;
        for (int i = 0; i < num_queries; i++) {
            Query<DIM> q = workload[i];
            auto query_start = std::chrono::high_resolution_clock::now();
            SumVisitor<DIM> visitor(1);
            engine.Execute(q, visitor);
            aggregate += visitor.sum;
            results.push_back(visitor.sum);
            auto query_end = std::chrono::high_resolution_clock::now();
            long tot_scanned = engine.ScannedPoints();
            prev = tot_scanned;
            query_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(query_end-query_start).count());
        }
    }
    auto finish = std::chrono::high_resolution_clock::now();
    auto tt = std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count();
    if (visitor_type == std::string("aggregate")) {
        std::cout << "Total aggregate returned by queries: " << aggregate << std::endl;
    } else if (visitor_type == "sum") {
        std::cout << "Sum returned by queries: " << aggregate << std::endl;
    } else {
        std::cout << "Total points returned by queries: " << total << std::endl;
    }
    std::cout << "Total queries: " << num_queries << std::endl;
    std::cout << "Avg range query time (ns): " << tt / ((double) num_queries) << std::endl;
    std::cout << "Total points scanned: " << engine.ScannedRangePoints() << " (range), "
        << engine.ScannedListPoints() << " (list)" << std::endl;    
    /*for (auto r: results) {
        std::cout << r << std::endl;
    } */   

    std::string savefile = GetWithDefault(flags, "save", "");
    if (!savefile.empty()) {
        auto results = std::ofstream(savefile);
        assert (results.is_open());
        results << "timestamp,name,dataset,workload_file,num_queries,"
            << "visitor,outlier_file,num_outliers,mapping_file,target_bucket_file,"
            << "rewriter_file,indexer_size,"
            << "total_pts,aggregate_result,avg_query_time,points_scanned" << std::endl;
        results << std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()
                << "," << GetWithDefault(flags, "name", "") 
                << "," << GetRequired(flags, "dataset")
                << "," << GetRequired(flags, "workload")
                << "," << num_queries
                << "," << visitor_type
                << "," << GetWithDefault(flags, "outlier-list", "")
                << "," << num_outliers
                << "," << GetWithDefault(flags, "indexer-spec", "")
                << "," << GetWithDefault(flags, "rewriter", "")
                << "," << indexer_size_bytes
                << "," << total
                << "," << aggregate
                << "," << tt / ((double)num_queries)
                << "," << engine.ScannedRangePoints()
                << "," << engine.ScannedListPoints()
            << std::endl; 
        results.close();
    }
}
