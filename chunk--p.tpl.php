<?php

/**
 * @file
 * Paragraph chunk template.
 */

/**
 * @TODO: add documentation about variables.
 */

?>

<p class="<?php print $classes; ?>">
  <?php print check_plain(trim(preg_replace('/\s+/', ' ', $configuration['p']))); ?>
</p>
