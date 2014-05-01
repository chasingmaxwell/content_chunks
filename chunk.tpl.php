<?php

/**
 * @file
 * Default chunk template.
 *
 * Should never actually be used since each chunk should provide
 */

/**
 * @TODO: add documentation about variables.
 */

?>

<div class="<?php print $classes; ?>">
  This is the default chunk template. If you you're seeing this, the <em><?php print $chunk['#chunk_type']->module; ?></em> module has not provided a template file for the chunk type <em><?php print $chunk['#chunk_type']->name; ?></em>.
</div>

