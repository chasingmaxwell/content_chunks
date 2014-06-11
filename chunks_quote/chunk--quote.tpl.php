<?php

/**
 * @file
 * Quote chunk template.
 */

/**
 * @TODO: add documentation about variables.
 */

?>

<blockquote class="<?php print $classes; ?>"<?php print $cite_attribute; ?>>
  <p<?php if (isset($configuration['edit_in_place'])): ?> contenteditable<?php endif; ?>>
    <?php print filter_xss(trim(preg_replace('/(\s|<br>|<\/?div>)+/', ' ', $configuration['quote'])), array('a', 'b', 'em', 'i', 'strong', 'u')); ?>
  </p>
  <?php if (!empty($configuration['attribution'])): ?>
    <footer>
      <cite<?php if (isset($configuration['edit_in_place'])): ?> contenteditable<?php endif; ?>><?php print filter_xss(trim(preg_replace('/(\s|<br>|<\/?div>)+/', ' ', $configuration['attribution'])), array('a', 'b', 'em', 'i', 'strong', 'u')); ?></cite>
    </footer>
  <?php endif; ?>
</blockquote>
