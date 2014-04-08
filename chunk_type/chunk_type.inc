<?

/**
 * @file
 * Provides the ChunkType class.
 */

class ChunkType {
  public $name;
  public $module;
  public $form_id;
  public $theme_hook;

  function __contruct($name, $module) {
    $this->name = $name;
    $this->module = $module;
    $this->form_id = $module . '_' . $name . '_chunk_form';
    $this->theme_hook = $module . '_' . $name . '_chunk';
  }

  public function getForm() {
    return drupal_get_form($this->form_id);
  }
}

