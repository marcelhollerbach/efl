evas_suite_src = [
  'evas_suite.c',
  'evas_test_new.c',
  'evas_test_init.c',
  'evas_test_object.c',
  'evas_test_object_smart.c',
  'evas_test_textblock.c',
  'evas_test_text.c',
  'evas_test_callbacks.c',
  'evas_test_render_engines.c',
  'evas_test_filters.c',
  'evas_test_image.c',
  'evas_test_mesh.c',
  'evas_test_mask.c',
  'evas_test_evasgl.c',
  'evas_test_matrix.c',
  'evas_test_focus.c',
  'evas_test_events.c',
  'evas_tests_helpers.h',
  'evas_suite.h'
]

evas_suite = executable('evas_suite',
  evas_suite_src,
  dependencies: [evas_bin, evas, ecore_evas, dl, check],
  include_directories: include_directories(join_paths('..', '..', 'modules', 'evas', 'engines', 'buffer')),
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('evas-suite', evas_suite,
  env : test_env
)
