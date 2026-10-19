// ʕ •ᴥ•ʔ✿ Custom_GetTotalMapMarkerCount pushes map_marker_total. That is spawn count plus waypoint count stored before the limit trim. ✿ ʕ •ᴥ•ʔ
int native_get_total_map_marker_count(void* state) {
    g_push_num(state, static_cast<double>(map_marker_total));
    return 1;
}
